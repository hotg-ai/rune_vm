//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <rune_vm/RuneVm.hpp>
#include <inference/ModelManager.hpp>

struct M3Environment;

namespace rune_vm_internal {
    class Wasm3Engine: public rune_vm::IEngine {
    public:
        Wasm3Engine(const rune_vm::ILogger::CPtr& logger, const inference::ModelManager::Ptr& modelManager);

    private:
        // rune_vm::IEngine
        [[nodiscard]] rune_vm::IRuntime::Ptr createRuntime(
            const std::optional<uint32_t> optStackSizeBytes,
            const std::optional<uint32_t> optMemoryLimit) final;

        // data
        rune_vm::LoggingModule m_log;
        std::shared_ptr<M3Environment> m_environment;
        inference::ModelManager::Ptr m_modelManager;
    };
}
