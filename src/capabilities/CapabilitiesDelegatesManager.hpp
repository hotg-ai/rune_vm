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
    using TDelegateFactory = std::function<rune_vm::capabilities::IDelegate::Ptr(const rune_vm::ILogger::CPtr&)>;
    extern std::vector<TDelegateFactory> g_defaultDelegateFactories;

    class CapabilitiesDelegatesManager : public rune_vm::capabilities::IContext {
    public:
        CapabilitiesDelegatesManager(
            const rune_vm::ILogger::CPtr& logger,
            const std::vector<rune_vm::capabilities::IDelegate::Ptr>& delegates);

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
            TKeyToParameterMap m_parameters;
            bool m_availability;
        };

        // data
        rune_vm::LoggingModule m_log;
        std::unordered_map<rune_vm::capabilities::Capability, rune_vm::capabilities::IDelegate::Ptr> m_delegates;
        std::unordered_map<rune_vm::capabilities::TId, CapabilityData> m_capabilityData;

    };

}


