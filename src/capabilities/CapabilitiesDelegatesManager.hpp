//
// Created by Kirill Delimbetov - github.com/delimbetov - on 29.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <rune_vm/Capabilities.hpp>
#include <rune_vm/Log.hpp>

namespace rune_vm_internal {

    class CapabilitiesDelegatesManager : public rune_vm::capabilities::IContext {
    public:
        using Ptr = std::shared_ptr<CapabilitiesDelegatesManager>;

        CapabilitiesDelegatesManager(
            const rune_vm::ILogger::CPtr& logger,
            const std::vector<rune_vm::capabilities::IDelegate::Ptr>& delegates);

        // host function helpers
        [[nodiscard]] std::optional<rune_vm::capabilities::TId> createCapability(
            const rune_vm::TRuneId runeId,
            const rune_vm::capabilities::Capability capability);
        bool setCapabilityParam(
            const rune_vm::TRuneId runeId,
            const rune_vm::capabilities::TId capabilityId,
            const rune_vm::capabilities::TKey& key,
            const rune_vm::capabilities::Parameter& parameter) noexcept;
        [[nodiscard]] bool getInput(
            const rune_vm::TRuneId runeId,
            const rune_vm::DataView<uint8_t> buffer,
            const rune_vm::capabilities::TId capabilityId) noexcept;

    private:
        // capabilities::IContext
        bool setCapabilityAvailability(
            const rune_vm::capabilities::TId capabilityId,
            const bool available) noexcept final;

        [[nodiscard]] std::optional<TKeyToParameterMap> getParamsForCapability(
            const rune_vm::capabilities::TId capabilityId) const noexcept final;
        [[nodiscard]] std::optional<rune_vm::capabilities::Parameter> getParamForCapabilityForKey(
            const rune_vm::capabilities::TId capabilityId,
            const rune_vm::capabilities::TKey& key) const noexcept final;
        [[nodiscard]] TCapabilityIdToDataMap getCapabilityIdToDataMap() const final;

        // data
        rune_vm::LoggingModule m_log;
        std::unordered_map<rune_vm::capabilities::Capability, rune_vm::capabilities::IDelegate::Ptr> m_delegates;
        TCapabilityIdToDataMap m_capabilityData;
        rune_vm::capabilities::TId m_idCounter;
    };

}


