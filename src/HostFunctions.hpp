//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <cstdint>
#include <rune_vm/Capabilities.hpp>
#include <rune_vm/Log.hpp>
#include <capabilities/CapabilitiesDelegatesManager.hpp>
#include <inference/ModelManager.hpp>
#include <OutputManager.hpp>
#include <RuneInterop.hpp>

namespace rune_vm_internal::host_functions {
    class HostContext {
    public:
        HostContext(
            const rune_vm::ILogger::CPtr& logger,
            CapabilitiesDelegatesManager::Ptr&& capabilitiesManager,
            const inference::ModelManager::Ptr& modelManager);

        const rune_vm::LoggingModule& log() const noexcept;
        const CapabilitiesDelegatesManager::Ptr& capabilitiesManager() noexcept;
        const CapabilitiesDelegatesManager::Ptr& capabilitiesManager() const noexcept;
        const inference::ModelManager::Ptr& modelManager() noexcept;
        OutputManager& outputManager() noexcept;

    private:
        // data
        rune_vm::LoggingModule m_log;
        CapabilitiesDelegatesManager::Ptr m_capabilitiesManager;
        inference::ModelManager::Ptr m_modelManager;
        OutputManager m_outputManager;
    };

    // Host functions
    using TCapabilityId = rune_vm::capabilities::TId;
    using TModelId = inference::TModelId;
    using TResult = rune_interop::TIntType;
    using TOutputId = TOutputId;
    static_assert(std::is_same_v<TCapabilityId, rune_interop::TIntType>);

    // // Setup helpers
    TCapabilityId requestCapability(HostContext* context, const rune_interop::Capability capabilityType) noexcept;
    TResult requestCapabilitySetParam(
        HostContext* context,
        const TCapabilityId capabilityId,
        const rune_vm::DataView<const uint8_t> key,
        const rune_vm::DataView<const uint8_t> value,
        const rune_interop::ValueType valueType) noexcept;

    // // Input helpers
    TResult requestProviderResponse(
        HostContext* context,
        const rune_vm::DataView<uint8_t> buffer,
        const TCapabilityId capabilityId) noexcept;

    // // Execution helpers
    TModelId tfmPreloadModel(
        HostContext* context,
        const rune_vm::DataView<const uint8_t> model,
        const uint32_t inputs,
        const uint32_t outputs) noexcept;
    TResult tfmModelInvoke(
        HostContext* context,
        const TModelId modelId,
        const rune_vm::DataView<const uint8_t> input,
        const rune_vm::DataView<uint8_t> output) noexcept;

    // // Output helpers
    TOutputId requestOutput(HostContext* context, const rune_interop::OutputType outputType) noexcept;
    TResult consumeOutput(
        HostContext* context,
        const TOutputId outputId,
        const rune_vm::DataView<const uint8_t> buffer) noexcept;

    // // Debug helpers
    TResult debug(HostContext* context, const rune_vm::DataView<const char> message) noexcept;

    // Link wasm names to actual functions
    template<auto name>
    constexpr auto nameToFunctionMap() noexcept {
        constexpr auto stringViewName = std::string_view(name);

        if constexpr(stringViewName == rune_interop::host_function_rune_name::g_requestCapability)
            return requestCapability;
        else if constexpr(stringViewName == rune_interop::host_function_rune_name::g_requestCapabilitySetParam)
            return requestCapabilitySetParam;
        else if constexpr(stringViewName == rune_interop::host_function_rune_name::g_requestProviderResponse)
            return requestProviderResponse;
        else if constexpr(stringViewName == rune_interop::host_function_rune_name::g_tfmPreloadModel)
            return tfmPreloadModel;
        else if constexpr(stringViewName == rune_interop::host_function_rune_name::g_tfmModelInvoke)
            return tfmModelInvoke;
        else if constexpr(stringViewName == rune_interop::host_function_rune_name::g_requestOutput)
            return requestOutput;
        else if constexpr(stringViewName == rune_interop::host_function_rune_name::g_consumeOutput)
            return consumeOutput;
        else if constexpr(stringViewName == rune_interop::host_function_rune_name::g_debug)
            return debug;
        else {
            static_assert([](auto) { return false; }(stringViewName));
        }
    }
}
