//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <wasm3.h>
#include <Common.hpp>
#include <wasm_backends/wasm3/Wasm3Engine.hpp>
#include <wasm_backends/wasm3/Wasm3Runtime.hpp>

namespace rune_vm_internal {
    using namespace rune_vm;

    Wasm3Engine::Wasm3Engine(const rune_vm::ILogger::CPtr& logger, const inference::ModelManager::Ptr& modelManager)
        : m_log(logger, "Wasm3Engine")
        , m_environment(m3_NewEnvironment(), m3_FreeEnvironment)
        , m_modelManager(modelManager) {
        CHECK_THROW(m_environment);
        CHECK_THROW(m_modelManager);
        m_log.log(Severity::Debug, "Wasm3Engine()");
    }

    // rune_vm::IEngine
    IRuntime::Ptr Wasm3Engine::createRuntime(
        const std::optional<uint32_t> optStackSizeBytes,
        const std::optional<uint32_t> optMemoryLimit) {
        return std::make_shared<Wasm3Runtime>(
            m_log.logger(),
            m_environment,
            optStackSizeBytes,
            optMemoryLimit,
            m_modelManager);
    }
}