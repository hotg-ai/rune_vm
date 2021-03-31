//
// Created by Kirill Delimbetov - github.com/delimbetov - on 31.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <inference/Inference.hpp>

struct TfLiteModel;
struct TfLiteInterpreterOptions;
struct TfLiteInterpreter;
struct TfLiteTensor;

namespace rune_vm_internal::inference {

    class TfLiteModel : public IModel {
    public:
        TfLiteModel(
            rune_vm::LoggingModule&& loggingModule,
            std::unique_ptr<TfLiteModel>&& model,
            std::unique_ptr<TfLiteInterpreter>&& interpreter);

        bool run(
            const rune_vm::DataView<rune_vm::DataView<const uint8_t>> inputs,
            const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs) noexcept;

    private:
        // data
        rune_vm::LoggingModule m_log;
        std::unique_ptr<TfLiteModel> m_model;
        // interpreter is model-specific
        std::unique_ptr<TfLiteInterpreter> m_interpreter;
    };

    class TfLiteRuntime : public IRuntime {
    public:
        TfLiteRuntime(const rune_vm::ILogger::CPtr& logger, const IRuntime::Options& options);

    private:
        // IRuntime
        bool run(
            const IModel::Ptr& model,
            const rune_vm::DataView<rune_vm::DataView<const uint8_t>> inputs,
            const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs) noexcept final;
        IModel::Ptr loadModel(
            const rune_vm::DataView<const uint8_t> model,
            const uint32_t inputs,
            const uint32_t outputs) final;

        // data
        rune_vm::LoggingModule m_log;
        std::unique_ptr<TfLiteInterpreterOptions> m_options;
    };
}


