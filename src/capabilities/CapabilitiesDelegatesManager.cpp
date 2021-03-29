//
// Created by Kirill Delimbetov - github.com/delimbetov - on 29.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <type_traits>
#include <fmt/format.h>
#include <capabilities/CapabilitiesDelegatesManager.hpp>

namespace {
    using namespace rune_vm;
}

namespace rune_vm_internal {
    CapabilitiesDelegatesManager::CapabilitiesDelegatesManager(
        const ILogger::CPtr& logger,
        const std::vector<capabilities::IDelegate::Ptr>& delegates)
        : m_log(logger, "CapabilitiesDelegatesMaanger")
        , m_delegates([&delegates] { 
                auto map = std::unordered_map<capabilities::Capability, capabilities::IDelegate::Ptr>();

                map.reserve(delegates.size());

                for(const auto& delegate: delegates) {
                    const auto supportedCapabilities = delegate->getSupportedCapabilities();

                    std::transform(
                        supportedCapabilities.begin(),
                        supportedCapabilities.end(),
                        std::inserter(map),
                        [this, &delegate](const auto capability) {
                            m_log.log(Severity::Info, fmt::format("Setting delegate for capability: {}", capability));
                            return std::make_pair(capability, delegate);
                        });
                }

                return map;
            }()) {
        m_log.log(Severity::Debug, fmt::format("CapabilitiesDelegatesManager() delegates set={}", m_delegates.size()));
    }

    // rune_vm::capabilities::IContext
    bool CapabilitiesDelegatesManager::setCapabilityAvailability(
        const capabilities::TId capabilityId,
        const bool available) noexcept {
        const auto [iter, found] = find(m_capabilityData, capabilityId);
        if(!found) {
            m_log.log(Severity::Debug, fmt::format("Failed to find capability data for id={}", capabilityId));
            return false;
        }

        m_log.log(
            Severity::Info,
            fmt::format("Setting availability of capability id={} to={}", capabilityId, available));
        iter->second.m_availability = available;

        return true;
    }

    std::optional<CapabilitiesDelegatesManager::TKeyToParameterMap> 
        CapabilitiesDelegatesManager::getParamsForCapability(const capabilities::TId capabilityId) const noexcept {
        const auto [iter, found] = find(m_capabilityData, capabilityId);
        if(!found) {
            m_log.log(Severity::Debug, fmt::format("Failed to find capability data for id={}", capabilityId));
            return std::nullopt;
        }

        return iter->second.m_parameters;
    }
    
    std::optional<capabilities::Parameter> CapabilitiesDelegatesManager::getParamForCapabilityForKey(
        const capabilities::TId capabilityId,
        const capabilities::TKey& key) const noexcept {
        const auto [iterId, foundId] = find(m_capabilityData, capabilityId);
        if(!foundId) {
            m_log.log(Severity::Debug, fmt::format("Failed to find capability data for id={}", capabilityId));
            return std::nullopt;
        }

        const auto [iterKey, foundKey] = find(iterId->second.m_parameters, key);
        if(!foundKey) {
            m_log.log(Severity::Debug, fmt::format("Failed to find parameter for id={} and key={}", capabilityId, key));
            return std::nullopt;
        }

        return iterKey->second;
    }
}