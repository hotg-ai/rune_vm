//
// Created by Kirill Delimbetov - github.com/delimbetov - on 03.04.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <inference/Inference.hpp>

namespace rune_vm_internal::inference {
    using TModelId = uint32_t;

    class ModelManager {
    public:
        using Ptr = std::shared_ptr<ModelManager>;

        ModelManager(const rune_vm::ILogger::CPtr& logger, IRuntime::Ptr runtime);

        std::optional<TModelId> loadModel(
            const rune_vm::DataView<const uint8_t> model,
            const uint32_t inputs,
            const uint32_t outputs) noexcept;
        bool runModel(
            const TModelId modelId,
            const rune_vm::DataView<const rune_vm::DataView<const uint8_t>> inputs,
            const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs) noexcept;

        std::optional<IModel::Ptr> getModel(const TModelId modelId) const noexcept;

        // false if no such model in cache
        bool eraseModel(const TModelId modelId) noexcept;

    private:
        rune_vm::LoggingModule m_log;
        IRuntime::Ptr m_runtime;
        std::unordered_map<TModelId, IModel::Ptr> m_models;
        TModelId m_idCounter;
    };
}



