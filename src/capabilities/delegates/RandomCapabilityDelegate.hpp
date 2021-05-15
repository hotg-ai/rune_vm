//
// Created by Kirill Delimbetov - github.com/delimbetov - on 29.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <array>
#include <random>
#include <unordered_map>
#include <capabilities/CapabilitiesDelegatesManager.hpp>

namespace rune_vm_internal {
    class RandomCapabilityDelegate : public rune_vm::capabilities::IDelegate {
    public:
        static constexpr auto g_supportedCapabilities = std::array{rune_vm::capabilities::Capability::Rand};

        RandomCapabilityDelegate(const rune_vm::ILogger::CPtr& logger);

    private:
        // rune_vm::capabilities::IDelegate
        [[nodiscard]] TCapabilitiesSet getSupportedCapabilities() const noexcept final;
        [[nodiscard]] bool requestCapability(
            const rune_vm::TRuneId runeId,
            const rune_vm::capabilities::Capability capability,
            const rune_vm::capabilities::TId newCapabilityId) noexcept final;
        [[nodiscard]] bool requestCapabilityParamChange(
            const rune_vm::TRuneId runeId,
            const rune_vm::capabilities::TId capabilityId,
            const rune_vm::capabilities::TKey& key,
            const rune_vm::capabilities::Parameter& parameter) noexcept final;
        [[nodiscard]] bool requestRuneInputFromCapability(
            const rune_vm::TRuneId runeId,
            const rune_vm::DataView<uint8_t> buffer,
            const rune_vm::capabilities::TId capabilityId) noexcept final;

        // Internal data structs
        struct CapabilityIdData {
            std::mt19937 m_engine;
            std::pair<float, float> m_range;
        };

        // data
        rune_vm::LoggingModule m_log;
        std::random_device m_randomDevice;
        std::unordered_map<rune_vm::capabilities::TId, CapabilityIdData> m_engines;
        TCapabilitiesSet m_supportedCapabilities;
    };
}
