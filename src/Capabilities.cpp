//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include "Capabilities.hpp"

namespace rune_vm_internal {
    constexpr Capability capabilityFromInt(const uint32_t capability) {
        switch(capability) {
            case 1:
                return Capability::Rand;
            case 2:
                return Capability::Sound;
            case 3:
                return Capability::Accel;
            case 4:
                return Capability::Image;
            case 5:
                return Capability::Raw;
        }

        CHECK_THROW(false);
    }
}