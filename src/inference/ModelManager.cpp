//
// Created by Kirill Delimbetov - github.com/delimbetov - on 03.04.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <fmt/format.h>
#include <Common.hpp>
#include <inference/ModelManager.hpp>

namespace rune_vm_internal::inference {
    using namespace rune_vm;

    ModelManager::ModelManager(const rune_vm::ILogger::CPtr& logger, IRuntime::Ptr runtime)
        : m_log(logger, "ModelManager")
        , m_runtime(std::move(runtime))
        , m_idCounter(0) {
        CHECK_THROW(m_runtime);
        m_log.log(Severity::Debug, "ModelManager()");
    }

    std::optional<TModelId> ModelManager::loadModel(
        const rune_vm::DataView<const uint8_t> model,
        const uint32_t inputs,
        const uint32_t outputs) noexcept {
        try {
            auto loadedModel = m_runtime->loadModel(model, inputs, outputs);
            const auto modelId = m_idCounter;

            // cache
            CHECK_THROW(loadedModel); // just in case
            m_models[modelId] = loadedModel;

            ++m_idCounter;

            if(m_idCounter == std::numeric_limits<decltype(m_idCounter)>::max()) {
                m_log.log(
                    Severity::Warning,
                    fmt::format("Model id counter has hit max value={}. Next allocation will overflow it", m_idCounter));
            }

            m_log.log(
                Severity::Info,
                fmt::format("New model of size={} allocated with id={}", model.m_size, modelId));
            return modelId;
        } catch(const std::exception& e) {
            m_log.log(Severity::Error, fmt::format("Failed to load model of size={}", model.m_size));
            return std::nullopt;
        }
    }

    bool ModelManager::runModel(
        const TModelId modelId,
        const rune_vm::DataView<const rune_vm::DataView<const uint8_t>> inputs,
        const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs) noexcept {
        const auto optModel = getModel(modelId);
        if(!optModel) {
            m_log.log(Severity::Warning, fmt::format("Failed to find model id={}", modelId));
            return false;
        }

        return m_runtime->run(*optModel, inputs, outputs);
    }

    std::optional<IModel::Ptr> ModelManager::getModel(const TModelId modelId) const noexcept {
        const auto [iter, found] = find(m_models, modelId);
        if(!found) {
            m_log.log(Severity::Warning, fmt::format("Failed to find model id={}", modelId));
            return std::nullopt;
        }

        return iter->second;
    }

    bool ModelManager::eraseModel(const TModelId modelId) noexcept {
        return m_models.erase(modelId) > 0;
    }
}