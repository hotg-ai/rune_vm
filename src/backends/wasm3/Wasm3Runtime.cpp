//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <fstream>
#include <fmt/format.h>
// wasm3
#include <m3_env.h>
#include <wasm3.h>
//
#include <mio/mmap.hpp>
#include <Common.hpp>
#include <backends/wasm3/Wasm3Rune.hpp>
#include <backends/wasm3/Wasm3Runtime.hpp>

namespace {
    using namespace rune_vm;

    auto checkM3Error(const LoggingModule& log, M3Runtime& runtime, const M3Result& result) {
        if(result != m3Err_none) {
            auto info = M3ErrorInfo();

            m3_GetErrorInfo(&runtime, &info);
            CHECK_THROW(info.file && info.message);
            log.log(
                Severity::Error,
                fmt::format("M3 function has failed: file={} line={} msg={}", info.file, info.line, info.message));
            CHECK_THROW(false);
        }
    }

    auto createRune(
        const LoggingModule& log,
        M3Environment& environment,
        M3Runtime& runtime,
        const DataView<const uint8_t> data) {
        auto rawModule = IM3Module();
        const auto parseResult = m3_ParseModule(&environment, &rawModule, data.m_data, data.m_size);
        checkM3Error(log, runtime, parseResult);
        CHECK_THROW(rawModule);
        auto loaded = std::make_shared<bool>(false);
        auto module = std::shared_ptr<M3Module>(
            rawModule,
            [loaded](IM3Module module) {
                // runtime cleans loaded modules
                if (!loaded) {
                    m3_FreeModule(module);
                }
            });

        // load into runtime
        const auto loadResult = m3_LoadModule(&runtime, module.get());
        checkM3Error(log, runtime, loadResult);

        *loaded = true;

        return std::make_shared<rune_vm_internal::Wasm3Rune>(log.logger(), std::move(module));
    }

    auto createRuntime(
        const LoggingModule& log,
        const std::shared_ptr<M3Environment>& environment,
        const std::optional<uint32_t> optStackSizeBytes,
        const std::optional<uint32_t> optMemoryLimit) {
        CHECK_THROW(environment);
        // TODO: add some proper default stack size deducer
        const auto stackSizeBytes = optStackSizeBytes.value_or(2u << 13);
        log.log(Severity::Info, fmt::format("Create wasm3 runtime with stackSizeBytes={}", stackSizeBytes));
        auto runtime = std::shared_ptr<M3Runtime>(
            m3_NewRuntime(environment.get(), stackSizeBytes, nullptr),
            m3_FreeRuntime);
        CHECK_THROW(runtime);

        if(optMemoryLimit) {
            log.log(Severity::Info, fmt::format("Set memory limit for wasm3 runtime={}", *optMemoryLimit));
            runtime->memoryLimit = *optMemoryLimit;
        }

        return runtime;
    }
}

namespace rune_vm_internal {
    using namespace rune_vm;

    Wasm3Runtime::Wasm3Runtime(
        const ILogger::CPtr& logger,
        const std::shared_ptr<M3Environment>& environment,
        const std::optional<uint32_t> optStackSizeBytes,
        const std::optional<uint32_t> optMemoryLimit)
        : m_log(logger, "Wasm3Runtime")
        , m_environment(environment)
        , m_runtime(createRuntime(m_log, m_environment, optStackSizeBytes, optMemoryLimit)) {
        m_log.log(Severity::Debug, "Wasm3Runtime()");
    }

    IRune::Ptr Wasm3Runtime::loadRune(const DataView<const uint8_t> data) {
        CHECK_THROW(data.m_data && data.m_size);
        m_log.log(Severity::Info, "loadRune from binary blob");
        return createRune(m_log, *m_environment, *m_runtime, data);
    }

    IRune::Ptr Wasm3Runtime::loadRune(const std::string_view fileName) {
        CHECK_THROW(!fileName.empty());
        m_log.log(Severity::Info, fmt::format("loadRune from fileName=", fileName));

        // load file
        auto errorCode = std::error_code();
        const auto mmapedFile = mio::make_mmap_source(fileName, errorCode);
        if(errorCode) {
            m_log.log(
                Severity::Error,
                fmt::format("Error mmaping file {}: code={} msg={}", fileName, errorCode.value(), errorCode.message()));
            CHECK_THROW(false);
        }

        const auto mmapDataView = DataView<const uint8_t>(
            reinterpret_cast<const uint8_t*>(mmapedFile.data()),
            mmapedFile.size());
        return createRune(m_log, *m_environment, *m_runtime, mmapDataView);
    }
}
