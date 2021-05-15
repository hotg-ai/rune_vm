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
#include <capabilities/delegates/DelegateFactory.hpp>
#include <capabilities/CapabilitiesDelegatesManager.hpp>
#include <wasm_backends/wasm3/Wasm3Rune.hpp>
#include <wasm_backends/wasm3/Wasm3Runtime.hpp>
#include <wasm_backends/wasm3/Wasm3Common.hpp>

namespace {
    using namespace rune_vm;
    using namespace rune_vm_internal;

    template<typename... TDeleter>
    auto createRune(
        const LoggingModule& log,
        TRuneId& runeIdCounterRef,
        M3Environment& environment,
        std::shared_ptr<M3Runtime> runtime,
        const inference::ModelManager::Ptr& modelManager,
        const std::vector<rune_vm::capabilities::IDelegate::Ptr>& delegates,
        const DataView<const uint8_t> data,
        TDeleter&&... deleter) {
        if(delegates.empty())
            log.log(Severity::Warning, "No delegates passed - that's very likely an error");

        auto rawModule = IM3Module();
        checkedCall(
            log,
            runtime,
            m3_ParseModule,
            &environment,
            &rawModule,
            data.m_data,
            data.m_size);
        CHECK_THROW(rawModule);
        auto loaded = std::make_shared<bool>(false);
        auto module = std::shared_ptr<M3Module>(
            rawModule,
            [loaded](IM3Module module) {
                // runtime cleans loaded modules
                if (!*loaded) {
                    m3_FreeModule(module);
                }
            });

        // load into runtime
        checkedCall(
            log,
            runtime,
            m3_LoadModule,
            runtime.get(),
            module.get());

        *loaded = true;

        // Create delegates manager
        const auto rune = std::shared_ptr<rune_vm_internal::Wasm3Rune>(
            new rune_vm_internal::Wasm3Rune(
                log.logger(),
                runeIdCounterRef,
                std::move(module),
                std::move(runtime),
                delegates,
                modelManager),
            std::forward<TDeleter>(deleter)...);

        ++runeIdCounterRef;

        if(runeIdCounterRef == std::numeric_limits<std::decay_t<decltype(runeIdCounterRef)>>::max()) {
            log.log(
                Severity::Warning,
                fmt::format("Rune id counter has hit max value={}. Next allocation will overflow it", runeIdCounterRef));
        }

        return rune;
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
        const std::optional<uint32_t> optMemoryLimit,
        const inference::ModelManager::Ptr& modelManager)
        : m_log(logger, "Wasm3Runtime")
        , m_runeIdCounter(0)
        , m_environment(environment)
        , m_runtime(createRuntime(m_log, m_environment, optStackSizeBytes, optMemoryLimit))
        , m_modelManager(modelManager) {
        m_log.log(Severity::Debug, "Wasm3Runtime()");
    }
    
    IRune::Ptr Wasm3Runtime::loadRune(
        const std::vector<rune_vm::capabilities::IDelegate::Ptr>& delegates,
        const DataView<const uint8_t> data) {
        CHECK_THROW(data.m_data && data.m_size);
        m_log.log(Severity::Info, "loadRune from binary blob");
        return createRune(m_log, m_runeIdCounter, *m_environment, m_runtime, m_modelManager, delegates, data);
    }

    IRune::Ptr Wasm3Runtime::loadRune(
        const std::vector<rune_vm::capabilities::IDelegate::Ptr>& delegates,
        const std::string_view fileName) {
        CHECK_THROW(!fileName.empty());
        m_log.log(Severity::Info, fmt::format("loadRune from fileName={}", fileName));

        // load file
        auto errorCode = std::error_code();
        auto mmapedFile = std::make_shared<mio::mmap_source>(mio::make_mmap_source(fileName, errorCode));
        if(errorCode) {
            m_log.log(
                Severity::Error,
                fmt::format("Error mmaping file {}: code={} msg={}", fileName, errorCode.value(), errorCode.message()));
            CHECK_THROW(false);
        }

        const auto mmapDataView = DataView<const uint8_t>(
            reinterpret_cast<const uint8_t*>(mmapedFile->data()),
            mmapedFile->size());
        return createRune(
            m_log,
            m_runeIdCounter,
            *m_environment,
            m_runtime,
            m_modelManager,
            delegates,
            mmapDataView,
            // wasm module uses memory it is created from
            [mmapedFile = std::move(mmapedFile)](auto* ptr) {
                delete ptr;
            });
    }

    std::vector<rune_vm::capabilities::Capability> Wasm3Runtime::getCapabilitiesWithDefaultDelegates() const noexcept {
        return getAllSupportedByDefaultCapabilities();
    }
}
