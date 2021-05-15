//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <rune_vm/RuneVm.hpp>

struct M3Environment;
struct M3Runtime;

namespace rune_vm_internal {
    class Wasm3Runtime: public rune_vm::IRuntime {
    public:
        Wasm3Runtime(
            const rune_vm::ILogger::CPtr& logger,
            const std::shared_ptr<M3Environment>& environment,
            const std::optional<uint32_t> optStackSizeBytes,
            const std::optional<uint32_t> optMemoryLimit,
            const inference::ModelManager::Ptr& modelManager);

    private:
        // IRune
        [[nodiscard]] rune_vm::IRune::Ptr loadRune(
            const std::vector<rune_vm::capabilities::IDelegate::Ptr>& delegates,
            const rune_vm::DataView<const uint8_t> data) final;
        [[nodiscard]] rune_vm::IRune::Ptr loadRune(
            const std::vector<rune_vm::capabilities::IDelegate::Ptr>& delegates,
            const std::string_view fileName) final;

        [[nodiscard]] std::vector<rune_vm::capabilities::Capability>
            getCapabilitiesWithDefaultDelegates() const noexcept final;

        // data
        rune_vm::LoggingModule m_log;
        rune_vm::TRuneId m_runeIdCounter;
        std::shared_ptr<M3Environment> m_environment;
        std::shared_ptr<M3Runtime> m_runtime;
        inference::ModelManager::Ptr m_modelManager;
    };
}


