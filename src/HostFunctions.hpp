//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <cstdint>
#include <Capabilities.hpp>
#include <RuneInterop.hpp>
#include <rune_vm/Capabilities.hpp>

namespace rune_vm_internal::host_functions {
    struct IHostContext {

    };

    // Host functions
    using TCapabilityId = rune_vm::capabilities::TId;
    using TModelId = rune_interop::TIntType;
    using TResult = rune_interop::TIntType;
    using TOutputId = rune_interop::TIntType;
    static_assert(std::is_same_v<TCapabilityId, rune_interop::TIntType>);

    // // Setup helpers
    TCapabilityId requestCapability(IHostContext* context, const rune_interop::Capability capabilityType) noexcept;
    TResult requestCapabilitySetParam(
        IHostContext* context,
        const TCapabilityId capabilityId,
        const rune_vm::DataView<const uint8_t> key,
        const rune_vm::DataView<const uint8_t> value,
        const rune_interop::ValueType valueType) noexcept;

    // // Input helpers
    TResult requestProviderResponse(
        IHostContext* context,
        const rune_vm::DataView<uint8_t> buffer,
        const TCapabilityId capabilityId) noexcept;

    // // Execution helpers
    TModelId tfmPreloadModel(
        IHostContext* context,
        const rune_vm::DataView<const uint8_t> model,
        const uint32_t inputs,
        const uint32_t outputs) noexcept;
    TResult tfmModelInvoke(
        IHostContext* context,
        const TModelId modelId,
        const rune_vm::DataView<const uint8_t> input,
        const rune_vm::DataView<uint8_t> output) noexcept;

    // // Output helpers
    TOutputId requestOutput(IHostContext* context, const uint32_t outputType) noexcept;
    TResult consumeOutput(
        IHostContext* context,
        const TOutputId outputId,
        const rune_vm::DataView<const uint8_t> buffer) noexcept;

    // // Debug helpers
    TResult debug(IHostContext* context, const rune_vm::DataView<const char> message) noexcept;

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


