//
// Created by Kirill Delimbetov - github.com/delimbetov - on 28.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <cstdint>
#include <string_view>
#include <rune_vm/Capabilities.hpp>

namespace rune_vm_internal::rune_interop {
    constexpr auto g_moduleName = "env";

    namespace host_function_rune_name {
        constexpr const char g_requestCapability[] = "request_capability";
        constexpr const char g_requestCapabilitySetParam[] = "request_capability_set_param";
        constexpr const char g_requestProviderResponse[] = "request_provider_response";
        constexpr const char g_tfmPreloadModel[] = "tfm_preload_model";
        constexpr const char g_tfmModelInvoke[] = "tfm_model_invoke";
        constexpr const char g_requestOutput[] = "request_output";
        constexpr const char g_consumeOutput[] = "consume_output";
        constexpr const char g_debug[] = "_debug";
    }

    namespace rune_function_name {
        constexpr const char g_manifest[] = "_manifest";
    }

    enum class OutputType: uint8_t {
        Serial,
        Ble,
        Pin,
        Wifi
    };

    using Capability = rune_vm::capabilities::Capability;
    using ValueType = rune_vm::capabilities::ValueType;

    enum ReturnCode: uint32_t {
        RC_Invalid,
        RC_InputError
    };

    // in
    using TIntType = uint32_t;

    template<typename T>
    constexpr T fromInt(const TIntType);

    // out

}


