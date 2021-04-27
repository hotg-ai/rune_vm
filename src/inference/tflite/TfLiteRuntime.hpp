//
// Created by Kirill Delimbetov - github.com/delimbetov - on 31.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <inference/Inference.hpp>

struct TfLiteModel;
struct TfLiteInterpreterOptions;
struct TfLiteInterpreter;

namespace rune_vm_internal::inference {
    class TfLiteLogger {
    public:
        TfLiteLogger(const rune_vm::ILogger::CPtr& logger);

        int log(const char* format, va_list args) const noexcept;

    private:
        rune_vm::LoggingModule m_log;
    };

    class TfLiteRuntimeModel : public IModel {
    public:
        TfLiteRuntimeModel(
            const rune_vm::ILogger::CPtr& logger,
            std::shared_ptr<TfLiteLogger>&& tfLogger,
            std::unique_ptr<const uint8_t[]>&& tfModelData,
            std::shared_ptr<TfLiteModel>&& tfModel,
            std::shared_ptr<TfLiteInterpreterOptions>&& tfOptions,
            std::shared_ptr<TfLiteInterpreter>&& tfInterpreter);

        bool run(
            const rune_vm::DataView<const rune_vm::DataView<const uint8_t>> inputs,
            const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs) noexcept;

    private:
        // IElement
        virtual void accept(IVisitor& visitor) noexcept final;

        // data
        rune_vm::LoggingModule m_log;
        std::shared_ptr<TfLiteLogger> m_tfLogger;
        std::unique_ptr<const uint8_t[]> m_modelData;
        std::shared_ptr<TfLiteModel> m_model;
        std::shared_ptr<TfLiteInterpreterOptions> m_options;
        // interpreter is model-specific
        std::shared_ptr<TfLiteInterpreter> m_interpreter;
    };

    class TfLiteRuntime : public IRuntime {
    public:
        TfLiteRuntime(const rune_vm::ILogger::CPtr& logger, const IRuntime::Options& options);

    private:
        // IRuntime
        bool run(
            const IModel::Ptr& model,
            const rune_vm::DataView<const rune_vm::DataView<const uint8_t>> inputs,
            const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs) noexcept final;
        IModel::Ptr loadModel(
            const rune_vm::DataView<const uint8_t> model,
            const uint32_t inputs,
            const uint32_t outputs) final;

        // data
        rune_vm::LoggingModule m_log;
        IRuntime::Options m_options;
    };
}


