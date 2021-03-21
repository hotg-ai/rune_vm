//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <cstdint>
#include <Capabilities.hpp>
#include <rune_vm/RuneVm.hpp>

namespace rune_vm_internal::host_functions {
    struct IHostContext {

    };

    enum class ValueType: uint8_t {
        Int32,
        Float32,
        Uint8,
        Int16
    };

    constexpr ValueType valueTypeFromInt(const uint32_t valueType);

    // Host functions
    using TCapabilityId = uint32_t;
    using TModelId = uint32_t;
    using TResult = uint32_t;
    using TOutputId = uint32_t;

    // // Setup helpers
    TCapabilityId requestCapability(const Capability capabilityType, IHostContext* context = nullptr);
    TResult requestCapabilitySetParam(
        const TCapabilityId capabilityId,
        const rune_vm::DataView<const uint8_t> key,
        const rune_vm::DataView<const uint8_t> value,
        const ValueType valueType,
        IHostContext* context = nullptr);

    // // Input helpers
    TResult requestProviderResponse(
        const rune_vm::DataView<uint8_t> buffer,
        const TCapabilityId capabilityId,
        IHostContext* context = nullptr);

    // // Execution helpers
    TModelId tfmPreloadModel(
        const rune_vm::DataView<const uint8_t> model,
        const uint32_t inputs,
        const uint32_t outputs,
        IHostContext* context = nullptr);
    TResult tfmModelInvoke(
        const TModelId modelId,
        const rune_vm::DataView<const uint8_t> input,
        const rune_vm::DataView<uint8_t> output,
        IHostContext* context = nullptr);

    // // Output helpers
    TOutputId requestOutput(const uint32_t outputType, IHostContext* context = nullptr);
    TResult consumeOutput(
        const TOutputId outputId,
        const rune_vm::DataView<const uint8_t> buffer,
        IHostContext* context = nullptr);

    // // Debug helpers
    TResult debug(const std::string_view message, IHostContext* context = nullptr);
}


