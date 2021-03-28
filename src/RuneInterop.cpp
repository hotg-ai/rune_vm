//
// Created by Kirill Delimbetov - github.com/delimbetov - on 28.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//


#include <RuneInterop.hpp>
#include <Common.hpp>

namespace rune_vm_internal::rune_interop {
    template<>
    constexpr ValueType fromInt<ValueType>(const TIntType valueType) {
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

    template<>
    constexpr OutputType fromInt<OutputType>(const TIntType outputType) {
        switch(outputType) {
            case 1:
                return OutputType::Serial;
            case 2:
                return OutputType::Ble;
            case 3:
                return OutputType::Pin;
            case 4:
                return OutputType::Wifi;
        }

        CHECK_THROW(false);
    }

    template<>
    constexpr Capability fromInt<Capability>(const TIntType capability) {
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