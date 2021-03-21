//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <Common.hpp>
#include <HostFunctions.hpp>

namespace rune_vm_internal::host_functions {
    constexpr ValueType valueTypeFromInt(const uint32_t valueType) {
        switch(valueType) {
            case 1:
                return ValueType::Int32;
            case 2:
                return ValueType::Float32;
            case 5:
                return ValueType::Uint8;
            case 6:
                return ValueType::Int16;
        }

        CHECK_THROW(false);
    }
}