//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <fmt/format.h>
#include <Common.hpp>
#include <HostFunctions.hpp>

namespace {
    using namespace rune_vm;

    template<typename T>
    bool readParameter(
        rune_vm_internal::host_functions::HostContext& context,
        capabilities::Parameter::Data& parameter,
        const rune_vm::DataView<const uint8_t> value) noexcept {
        if(value.m_size != sizeof(T)) {
            context.log().log(
                Severity::Error,
                fmt::format("requestCapabilitySetParam value size invalid: exp={} act={}", sizeof(T), value.m_size));
            return false;
        }

        auto casted = T();

        std::memcpy(&casted, value.m_data, sizeof(casted));
        parameter = casted;

        return true;
    }
}

namespace rune_vm_internal::host_functions {
    HostContext::HostContext(
        const rune_vm::ILogger::CPtr& logger,
        const rune_vm::TRuneId runeId,
        CapabilitiesDelegatesManager::Ptr&& capabilitiesManager,
        const inference::ModelManager::Ptr& modelManager)
        : m_log(logger, "HostContext")
        , m_runeId(runeId)
        , m_capabilitiesManager(std::move(capabilitiesManager))
        , m_modelManager(modelManager)
        , m_outputManager(logger) {
        CHECK_THROW(m_capabilitiesManager);
        CHECK_THROW(m_modelManager);
        m_log.log(Severity::Debug, "HostContext()");
    }

    rune_vm::TRuneId HostContext::runeId() const noexcept {
        return m_runeId;
    }

    const rune_vm::LoggingModule& HostContext::log() const noexcept {
        return m_log;
    }

    const CapabilitiesDelegatesManager::Ptr& HostContext::capabilitiesManager() noexcept {
        return m_capabilitiesManager;
    }

    const CapabilitiesDelegatesManager::Ptr& HostContext::capabilitiesManager() const noexcept {
        return m_capabilitiesManager;
    }

    const inference::ModelManager::Ptr& HostContext::modelManager() noexcept {
        return m_modelManager;
    }

    OutputManager& HostContext::outputManager() noexcept {
        return m_outputManager;
    }

    //
    TCapabilityId requestCapability(HostContext* context, const rune_interop::Capability capabilityType) noexcept {
        if(!context) {
            return rune_interop::RC_InputError;
        }

        context->log().log(Severity::Debug, fmt::format("requestCapability: capabilityType={}", capabilityType));
        const auto optCapabilityId = context->capabilitiesManager()->createCapability(context->runeId(), capabilityType);
        if(!optCapabilityId) {
            context->log().log(Severity::Error, fmt::format("Failed to create capability type={}", capabilityType));
            return rune_interop::RC_InputError;
        }

        context->log().log(Severity::Info, fmt::format("requestCapability result={}", *optCapabilityId));
        return *optCapabilityId;
    }

    TResult requestCapabilitySetParam(
        HostContext* context,
        const TCapabilityId capabilityId,
        const rune_vm::DataView<const uint8_t> key,
        const rune_vm::DataView<const uint8_t> value,
        const rune_interop::ValueType valueType) noexcept {
        if(!context) {
            return rune_interop::RC_InputError;
        }

        context->log().log(
            Severity::Debug,
            fmt::format(
                "requestCapabilitySetParam: capabilityId={} valueType={}",
                capabilityId,
                valueType));

        if(!key.m_data || key.m_size == 0) {
            context->log().log(Severity::Error, "requestCapabilitySetParam key invalid");
            return rune_interop::RC_InputError;
        }

        if(!value.m_data || value.m_size == 0) {
            context->log().log(Severity::Error, "requestCapabilitySetParam value invalid");
            return rune_interop::RC_InputError;
        }

        const auto keyStr = std::string(key.begin(), key.end());
        auto parameter = capabilities::Parameter::Data();
        switch(valueType) {
            case rune_interop::ValueType::Uint8: {
                if(!readParameter<uint8_t>(*context, parameter, value))
                    return rune_interop::RC_InputError;

                break;
            }
            case rune_interop::ValueType::Int16: {
                if(!readParameter<int16_t>(*context, parameter, value))
                    return rune_interop::RC_InputError;

                break;
            }
            case rune_interop::ValueType::Int32: {
                if(!readParameter<int32_t>(*context, parameter, value))
                    return rune_interop::RC_InputError;

                break;
            }
            case rune_interop::ValueType::Float32: {
                static_assert(sizeof(float) == 4);
                if(!readParameter<float>(*context, parameter, value))
                    return rune_interop::RC_InputError;

                break;
            }
            default:
                context->log().log(Severity::Error, "requestCapabilitySetParam unknown value type");
                return rune_interop::RC_InputError;
        }

        const auto setResult = context->capabilitiesManager()->setCapabilityParam(
            context->runeId(),
            capabilityId,
            keyStr,
            capabilities::Parameter{std::move(parameter)});
        if(!setResult) {
            context->log().log(Severity::Error, "setCapabilityParam failed");
            return rune_interop::RC_InputError;
        }

        return 0;
    }

    // // Input helpers
    TResult requestProviderResponse(
        HostContext* context,
        const rune_vm::DataView<uint8_t> buffer,
        const TCapabilityId capabilityId) noexcept {
        if(!context) {
            return rune_interop::RC_InputError;
        }

        context->log().log(
            Severity::Debug,
            fmt::format("requestProviderResponse: capabilityId={}", capabilityId));

        if(!buffer.m_data || buffer.m_size == 0) {
            context->log().log(Severity::Error, "requestProviderResponse buffer invalid");
            return rune_interop::RC_InputError;
        }

        const auto inputResult = context->capabilitiesManager()->getInput(context->runeId(), buffer, capabilityId);
        if(!inputResult) {
            context->log().log(Severity::Error, "requestProviderResponse failed to get input");
            return rune_interop::RC_InputError;
        }

        // TODO: consider input size might exceed actually written amount
        return buffer.m_size;
    }

    // // Execution helpers
    TModelId tfmPreloadModel(
        HostContext* context,
        const rune_vm::DataView<const uint8_t> modelData,
        const uint32_t inputs,
        const uint32_t outputs) noexcept {
        if(!context) {
            return rune_interop::RC_InputError;
        }

        context->log().log(Severity::Debug, fmt::format("tfmPreloadModel: model size={}", modelData.m_size));

        if(!modelData.m_data || modelData.m_size == 0) {
            context->log().log(Severity::Error, "tfmPreloadModel model invalid");
            return rune_interop::RC_InputError;
        }

        const auto optModelId = context->modelManager()->loadModel(modelData, inputs, outputs);
        if(!optModelId) {
            context->log().log(
                Severity::Error,
                fmt::format("Failed to tfmPreloadModel: model size={}", modelData.m_size));
            return rune_interop::RC_InputError;
        }

        context->log().log(
            Severity::Info,
            fmt::format("tfmPreloadModel: model size={} loaded with id={}", modelData.m_size, *optModelId));

        return *optModelId;
    }

    TResult tfmModelInvoke(
        HostContext* context,
        const TModelId modelId,
        const rune_vm::DataView<const uint8_t> input,
        const rune_vm::DataView<uint8_t> output) noexcept {
        if(!context) {
            return rune_interop::RC_InputError;
        }

        context->log().log(
            Severity::Debug,
            fmt::format(
                "tfmModelInvoke: model id={} input size={} outputs size={}",
                modelId,
                input.m_size,
                output.m_size));

        if(!input.m_data || input.m_size == 0) {
            context->log().log(Severity::Error, "tfmModelInvoke: input buffer invalid");
            return rune_interop::RC_InputError;
        }

        if(!output.m_data || output.m_size == 0) {
            context->log().log(Severity::Error, "tfmModelInvoke: output buffer invalid");
            return rune_interop::RC_InputError;
        }

        auto viewCopy = output;
        const auto runResult = context->modelManager()->runModel(modelId, {&input, 1}, {&viewCopy, 1});
        if(!runResult) {
            context->log().log(Severity::Error, fmt::format("tfmModelInvoke: failed to run model id={}", modelId));
            return rune_interop::RC_InputError;
        }

        return 0;
    }

    // // Output helpers
    TOutputId requestOutput(HostContext* context, const rune_interop::OutputType outputType) noexcept {
        if(!context) {
            return rune_interop::RC_InputError;
        }

        context->log().log(Severity::Debug, fmt::format("requestOutput: outputType={}", outputType));
        const auto optOutputId = context->outputManager().requestOutput(outputType);
        if(!optOutputId) {
            context->log().log(
                Severity::Error,
                fmt::format("requestOutput: failed to allocate output type={}", outputType));
            return rune_interop::RC_InputError;
        }

        context->log().log(Severity::Info, fmt::format("requestOutput: allocated output id={}", *optOutputId));
        return *optOutputId;
    }

    TResult consumeOutput(
        HostContext* context,
        const TOutputId outputId,
        const rune_vm::DataView<const uint8_t> buffer) noexcept {
        if(!context) {
            return rune_interop::RC_InputError;
        }

        context->log().log(
            Severity::Debug,
            fmt::format("consumeOutput: outputId={} buffer size={}", outputId, buffer.m_size));

        if(!buffer.m_data || buffer.m_size == 0) {
            context->log().log(Severity::Error, "consumeOutput: input buffer invalid");
            return rune_interop::RC_InputError;
        }

        const auto saveResult = context->outputManager().saveOutput(outputId, buffer);
        if(!saveResult) {
            context->log().log(Severity::Error, "consumeOutput: failed to save result");
            return rune_interop::RC_InputError;
        }

        return 0;
    }

    TResult debug(HostContext* context, const rune_vm::DataView<const char> message) noexcept {
        if(!context) {
            return rune_interop::RC_InputError;
        }

        if(!message.m_data || message.m_size == 0) {
            context->log().log(Severity::Error, "host_functions::debug: input message invalid");
            return rune_interop::RC_InputError;
        }

        context->log().log(
            Severity::Debug,
            fmt::format("host_functions::debug: message={}", std::string_view(message.m_data, message.m_size)));

        return 0;
    }

    TModelId runeModelLoad(HostContext *context, const rune_vm::DataView<const char> mimeType,
                           const rune_vm::DataView<const uint8_t> modelData,
                           const rune_vm::DataView<const rune_vm::DataView<const char> > inputs,
                           const rune_vm::DataView<const rune_vm::DataView<const char> > outputs) noexcept
    {
        auto modelId = tfmPreloadModel(context, modelData, inputs.m_size, outputs.m_size);

        if (modelId >= 0) {
            auto model = context->modelManager()->getModel(modelId).value_or(nullptr);

            if (model) {
                model->inputDescriptors.clear();
                model->inputDescriptors.reserve(inputs.m_size);

                model->outputDescriptors.clear();
                model->outputDescriptors.reserve(outputs.m_size);

                for (const auto &input: inputs) {
                    model->inputDescriptors.push_back(input);
                }

                for (const auto &output: outputs) {
                    model->outputDescriptors.push_back(output);
                }
            }
        }

        return modelId;
    }

    TResult runeModelInfer(HostContext *context,
                           const TModelId modelId,
                           const void* inputs,
                           void* outputs) noexcept
    {
        context->log().log(
            Severity::Debug,
            fmt::format(
                "tfmModelInvoke: model id={}",
                modelId));

        auto model = context->modelManager()->getModel(modelId).value_or(nullptr);

        if (model) {
            std::vector<rune_vm::DataView<const uint8_t>> inputTensors(model->inputDescriptors.size(), DataView<const uint8_t>());
            std::vector<rune_vm::DataView<uint8_t>> outputTensors(model->outputDescriptors.size(), DataView<uint8_t>());

            uint8_t* inputTensorBasePointer = static_cast<uint8_t*>(const_cast<void*>(inputs));
            uint8_t* outputTensorBasePointer = static_cast<uint8_t*>(outputs);

            for (size_t i = 0; i < inputTensors.size(); i++) {
                size_t currentTensorBytes = model->inputDescriptors[i].byteCount();
                inputTensors[i].m_data = inputTensorBasePointer;
                inputTensors[i].m_size = currentTensorBytes;

                inputTensorBasePointer += currentTensorBytes;
            }

            for (size_t i = 0; i < outputTensors.size(); i++) {
                size_t currentTensorBytes = model->outputDescriptors[i].byteCount();
                outputTensors[i].m_data = outputTensorBasePointer;
                outputTensors[i].m_size = currentTensorBytes;

                outputTensorBasePointer += currentTensorBytes;
            }

            const auto runResult = context->modelManager()->runModel(modelId,
                                                                     rune_vm::DataView<const rune_vm::DataView<const uint8_t>>(inputTensors.data(), inputTensors.size()),
                                                                     rune_vm::DataView<rune_vm::DataView<uint8_t>> (outputTensors.data(), outputTensors.size()));
            if(!runResult) {
                context->log().log(Severity::Error, fmt::format("tfmModelInvoke: failed to run model id={}", modelId));
                return rune_interop::RC_InputError;
            }

            return 0;
        }

        return rune_interop::RC_InputError;
    }

}
