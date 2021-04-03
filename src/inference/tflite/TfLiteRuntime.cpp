//
// Created by Kirill Delimbetov - github.com/delimbetov - on 31.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <tensorflow/lite/c/c_api.h>
#include <fmt/format.h>
#include <Common.hpp>
#include <inference/tflite/TfLiteRuntime.hpp>

namespace rune_vm_internal::inference {
    using namespace rune_vm;

    struct IVisitor {
        virtual void visit(TfLiteRuntimeModel& el) noexcept = 0;
    };

    struct RunVisitor: IVisitor {
        RunVisitor(
            const rune_vm::DataView<rune_vm::DataView<const uint8_t>> inputs,
            const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs)
            : m_inputs(inputs)
            , m_outputs(outputs) {}

        void visit(TfLiteRuntimeModel& model) noexcept final {
            m_result = model.run(m_inputs, m_outputs);
        }

        bool result() const noexcept {
            return m_result;
        }

    private:
        rune_vm::DataView<rune_vm::DataView<const uint8_t>> m_inputs;
        rune_vm::DataView<rune_vm::DataView<uint8_t>> m_outputs;
        bool m_result;
    };

    //
    TfLiteLogger::TfLiteLogger(const rune_vm::ILogger::CPtr& logger)
        : m_log(logger, "TfLiteLogger") {
        m_log.log(Severity::Debug, "TfLiteLogger()");
    }

    int TfLiteLogger::log(const char* format, va_list args) const noexcept {
        auto buffer = std::array<std::string::value_type, 1024>();
        const auto size = vsnprintf(buffer.data(), sizeof(buffer), format, args);

        m_log.log(Severity::Error, std::string_view(buffer.data(), size));

        return size;
    }

    //
    TfLiteRuntimeModel::TfLiteRuntimeModel(
        const rune_vm::ILogger::CPtr& logger,
        std::shared_ptr<TfLiteLogger>&& tfLogger,
        std::shared_ptr<TfLiteModel>&& tfModel,
        std::shared_ptr<TfLiteInterpreterOptions>&& tfOptions,
        std::shared_ptr<TfLiteInterpreter>&& tfInterpreter)
        : m_log(logger, "TfLiteRuntimeModel")
        , m_tfLogger(std::move(tfLogger))
        , m_model(std::move(tfModel))
        , m_options(std::move(tfOptions))
        , m_interpreter(std::move(tfInterpreter)) {
        m_log.log(Severity::Debug, "TfLiteRuntimeModel()");
    }

    bool TfLiteRuntimeModel::run(
        const rune_vm::DataView<rune_vm::DataView<const uint8_t>> inputs,
        const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs) noexcept {
        try {
            // Write input into tensors
            m_log.log(
                Severity::Debug,
                fmt::format("run inputs={} outputs={}", inputs.m_size, outputs.m_size));
            const auto inputTensorCount = TfLiteInterpreterGetInputTensorCount(m_interpreter.get());
            const auto outputTensorCount = TfLiteInterpreterGetOutputTensorCount(m_interpreter.get());
            CHECK_THROW(inputs.m_data && inputs.m_size == inputTensorCount);
            CHECK_THROW(outputs.m_data && outputs.m_size == outputTensorCount);
            for(auto idx = 0u; idx < inputs.m_size; ++idx) {
                const auto input = inputs[idx];
                CHECK_THROW(input.m_data);

                // get tensor handle
                auto* tensor = TfLiteInterpreterGetInputTensor(m_interpreter.get(), idx);
                CHECK_THROW(tensor);
                const auto byteCount = TfLiteTensorByteSize(tensor);
                CHECK_THROW(byteCount == input.m_size);

                // copy input into tensor
                const auto copyResult = TfLiteTensorCopyFromBuffer(tensor, input.m_data, byteCount);
                CHECK_THROW(copyResult == kTfLiteOk);
            }

            // Run inference
            m_log.log(Severity::Debug, "Run inference");
            const auto inferenceResult = TfLiteInterpreterInvoke(m_interpreter.get());
            CHECK_THROW(inferenceResult == kTfLiteOk);

            // Read output
            m_log.log(Severity::Debug, "Read output");
            for(auto idx = 0u; idx < outputs.m_size; ++idx) {
                const auto output = outputs[idx];
                CHECK_THROW(output.m_data);

                // get tensor handle
                auto* tensor = TfLiteInterpreterGetOutputTensor(m_interpreter.get(), idx);
                CHECK_THROW(tensor);
                const auto byteCount = TfLiteTensorByteSize(tensor);
                CHECK_THROW(byteCount == output.m_size);

                // copy input into tensor
                const auto copyResult = TfLiteTensorCopyToBuffer(tensor, output.m_data, byteCount);
                CHECK_THROW(copyResult == kTfLiteOk);
            }
        } catch(const std::exception& e) {
            m_log.log(Severity::Error, fmt::format("Failed to run(): {}", e.what()));
            return false;
        }

        return true;
    }

    // IElement
    void TfLiteRuntimeModel::accept(IVisitor& visitor) noexcept {
        visitor.visit(*this);
    }

    //
    TfLiteRuntime::TfLiteRuntime(const rune_vm::ILogger::CPtr& logger, const IRuntime::Options& options)
        : m_log(logger, "TfLiteRuntime")
        , m_options(options) {
        m_log.log(Severity::Debug, "TfLiteRuntime()");
        m_log.log(Severity::Info, fmt::format("Creating runtime with num threads={}", m_options.m_numThreads));
        CHECK_THROW(options.m_numThreads > 0);
    }

    // IRuntime
    bool TfLiteRuntime::run(
        const IModel::Ptr& model,
        const rune_vm::DataView<rune_vm::DataView<const uint8_t>> inputs,
        const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs) noexcept {
        auto visitor = RunVisitor(inputs, outputs);

        model->accept(visitor);

        return visitor.result();
    }

    IModel::Ptr TfLiteRuntime::loadModel(
        const rune_vm::DataView<const uint8_t> model,
        const uint32_t inputs,
        const uint32_t outputs) {
        m_log.log(Severity::Info, fmt::format("Loading tf model of size={}", model.m_size));
        CHECK_THROW(model.m_data && model.m_size);
        auto tfModel = std::shared_ptr<TfLiteModel>(TfLiteModelCreate(model.m_data, model.m_size), TfLiteModelDelete);
        CHECK_THROW(tfModel);
        auto tfOptions = std::shared_ptr<TfLiteInterpreterOptions>(
            TfLiteInterpreterOptionsCreate(),
            TfLiteInterpreterOptionsDelete);
        CHECK_THROW(tfOptions);
        auto tfLogger = std::make_shared<TfLiteLogger>(m_log.logger());
        CHECK_THROW(tfLogger);

        TfLiteInterpreterOptionsSetErrorReporter(
            tfOptions.get(),
            [](void* userData, const char* format, va_list args) {
                reinterpret_cast<TfLiteLogger*>(userData)->log(format, args);
            },
            tfLogger.get());
        TfLiteInterpreterOptionsSetNumThreads(tfOptions.get(), m_options.m_numThreads);

        auto tfInterpreter = std::shared_ptr<TfLiteInterpreter>(
            TfLiteInterpreterCreate(tfModel.get(), tfOptions.get()),
            TfLiteInterpreterDelete);
        CHECK_THROW(tfInterpreter);

        const auto allocationResult = TfLiteInterpreterAllocateTensors(tfInterpreter.get());
        CHECK_THROW(allocationResult == kTfLiteOk);

        return std::make_shared<TfLiteRuntimeModel>(
            m_log.logger(),
            std::move(tfLogger),
            std::move(tfModel),
            std::move(tfOptions),
            std::move(tfInterpreter));
    }
}
