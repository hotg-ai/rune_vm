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
        CapabilitiesDelegatesManager(
            const rune_vm::ILogger::CPtr& logger,
            const std::vector<rune_vm::capabilities::IDelegate::Ptr>& delegates);

        [[nodiscard]] std::optional<rune_vm::capabilities::TId> createCapability(
            const rune_vm::capabilities::Capability capability);
        bool setCapabilityParam(
            const rune_vm::capabilities::TId capabilityId,
            const rune_vm::capabilities::TKey& key,
            const rune_vm::capabilities::Parameter& parameter) noexcept;
        bool getInput(
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

        // Internal data structs
        struct CapabilityData {
            CapabilityData(rune_vm::capabilities::IDelegate::Ptr owner);

            TKeyToParameterMap m_parameters;
            bool m_availability;

            rune_vm::capabilities::IDelegate::Ptr owner() const noexcept;

        private:
            rune_vm::capabilities::IDelegate::Ptr m_owner;
        };

        // data
        rune_vm::LoggingModule m_log;
        std::unordered_map<rune_vm::capabilities::Capability, rune_vm::capabilities::IDelegate::Ptr> m_delegates;
        std::unordered_map<rune_vm::capabilities::TId, CapabilityData> m_capabilityData;
        rune_vm::capabilities::TId m_idCounter;
    };

}


