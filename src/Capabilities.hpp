//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <cstdint>
#include <Common.hpp>

namespace rune_vm_internal {
    enum class Capability: uint8_t {
        Rand,
        Sound,
        Accel,
        Image,
        Raw
    };

    constexpr Capability capabilityFromInt(const uint32_t capability);
}


