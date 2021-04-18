//
// Created by Kirill Delimbetov - github.com/delimbetov - on 29.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <type_traits>
#include <io/FormatCapabilitiesParameter.hpp>
#include <Common.hpp>
#include <capabilities/CapabilitiesDelegatesManager.hpp>
#include <capabilities/delegates/DelegateFactory.hpp>

namespace {
    using namespace rune_vm;
    using namespace rune_vm_internal;

    std::optional<rune_vm::capabilities::IDelegate::Ptr> getDelegateForCapability(
        LoggingModule& log,
        std::unordered_map<rune_vm::capabilities::Capability, rune_vm::capabilities::IDelegate::Ptr>& delegates,
        const capabilities::Capability capability) {
        const auto [delegateIter, found] = find(delegates, capability);
        if(found)
            return delegateIter->second;

        log.log(
            Severity::Warning,
            fmt::format("No user declared delegate set for capability={}. Try to find default one", capability));
        const auto optDefaultDelegate = getDefaultDelegateImplementingCapability(log.logger(), capability);
        if(!optDefaultDelegate) {
            log.log(
                Severity::Error,
                fmt::format(
                    "Failed to find default delegate. Can't create capability={} without delegate set for it",
                    capability));
            return std::nullopt;
        }

        // if delegate was created we should add it to the map
        CHECK_THROW(*optDefaultDelegate);
        log.log(Severity::Info, fmt::format("Setting default delegate for capability: {}", capability));
        delegates.emplace(capability, *optDefaultDelegate);

        return *optDefaultDelegate;
    }
}

namespace rune_vm_internal {
    CapabilitiesDelegatesManager::CapabilitiesDelegatesManager(
        const ILogger::CPtr& logger,
        const std::vector<capabilities::IDelegate::Ptr>& delegates)
        : m_log(logger, "CapabilitiesDelegatesManager")
        , m_delegates([this, &delegates] {
                auto map = std::unordered_map<capabilities::Capability, capabilities::IDelegate::Ptr>();

                map.reserve(delegates.size());

                for(const auto& delegate: delegates) {
                    const auto supportedCapabilities = delegate->getSupportedCapabilities();

                    std::transform(
                        supportedCapabilities.begin(),
                        supportedCapabilities.end(),
                        std::inserter(map, map.begin()),
                        [this, &delegate](const auto capability) {
                            CHECK_THROW(delegate);
                            m_log.log(Severity::Info, fmt::format("Setting delegate for capability: {}", capability));
                            return std::make_pair(capability, delegate);
                        });
                }

                return map;
            }())
        , m_idCounter(0) {
        m_log.log(Severity::Debug, fmt::format("CapabilitiesDelegatesManager() delegates set={}", m_delegates.size()));
    }

    std::optional<capabilities::TId> CapabilitiesDelegatesManager::createCapability(
        const capabilities::Capability capability) {
        const auto optDelegate = getDelegateForCapability(m_log, m_delegates, capability);
        if(!optDelegate) {
            m_log.log(
                Severity::Error,
                fmt::format("Can't create capability={} without delegate set for it", capability));
            return std::nullopt;
        }

        auto& delegate = *optDelegate;
        const auto allocatedCapabilityId = m_idCounter + 1;
        const auto result = delegate->requestCapability(capability, allocatedCapabilityId);
        if(!result) {
            m_log.log(
                Severity::Error,
                fmt::format("Can't create capability={}: delegate denied request", capability));
            return std::nullopt;
        }

        m_idCounter = allocatedCapabilityId;

        const auto [capabilityDataIter, inserted] = m_capabilityData.insert_or_assign(
            allocatedCapabilityId,
            CapabilityData(delegate, capability));
        if(!inserted) {
            m_log.log(
                Severity::Warning,
                fmt::format("Capability id={} existed for some reason. Logic error or overflow?", allocatedCapabilityId));
        }

        m_log.log(
            Severity::Info,
            fmt::format("Capability={} created with id={}", capability, allocatedCapabilityId));

        if(m_idCounter == std::numeric_limits<decltype(m_idCounter)>::max()) {
            m_log.log(
                Severity::Warning,
                fmt::format("Capability id counter has hit max value={}. Next allocation will overflow it", m_idCounter));
        }

        return allocatedCapabilityId;
    }

    bool CapabilitiesDelegatesManager::setCapabilityParam(
        const capabilities::TId capabilityId,
        const capabilities::TKey& key,
        const capabilities::Parameter& parameter) noexcept {
        const auto [iterId, foundId] = find(m_capabilityData, capabilityId);
        if(!foundId) {
            m_log.log(Severity::Error, fmt::format("Failed to find capability data for id={}", capabilityId));
            return false;
        }

        // if the key is there already, value will not be updated
        auto [iter, inserted] = iterId->second.m_parameters.emplace(key, parameter);
        const auto requestResult = iterId->second.owner()->requestCapabilityParamChange(capabilityId, key, parameter);
        if(!requestResult) {
            m_log.log(
                Severity::Error,
                fmt::format(
                    "Delegate for capability id={} denied updating key={} to parameter={}",
                    capabilityId,
                    key,
                    parameter));

            // if request is denied, erase inserted param
            // in case it was not inserted (i.e. key was there already) state did not change so no need for rollback
            if(inserted)
                iterId->second.m_parameters.erase(iter);

            return false;
        }

        if(inserted) {
            m_log.log(
                Severity::Info,
                fmt::format("Failed to find parameter for id={} and key={} -> create it", capabilityId, key));
        }

        iter->second = std::move(parameter);

        if(!iterId->second.m_availability)
            m_log.log(Severity::Warning, "Setting param for capability which is not available currently");

        static_assert(std::is_nothrow_copy_assignable_v<std::decay_t<decltype(parameter)>>);
        m_log.log(
            Severity::Info,
            fmt::format("Capability parameter={} for id={} and key={} is set", parameter, capabilityId, key));

        return true;
    }

    bool CapabilitiesDelegatesManager::getInput(
        const DataView<uint8_t> buffer,
        const capabilities::TId capabilityId) noexcept {
        if(!buffer.m_data) {
            m_log.log(
                Severity::Error,
                fmt::format("Passed buffer for input of capability id={} is nullptr", capabilityId));
            return false;
        }

        const auto [iterId, foundId] = find(m_capabilityData, capabilityId);
        if(!foundId) {
            m_log.log(Severity::Error, fmt::format("Failed to find capability data for id={}", capabilityId));
            return false;
        }

        if(!iterId->second.m_availability) {
            m_log.log(Severity::Error, fmt::format("Requesting input from unavailable capability id={}", capabilityId));
            return false;
        }

        const auto requestResult = iterId->second.owner()->requestRuneInputFromCapability(buffer, capabilityId);
        if(!requestResult) {
            m_log.log(Severity::Error, fmt::format("Delegate for capability id={} denied input request", capabilityId));
            return false;
        }

        m_log.log(
            Severity::Info,
            fmt::format("Input for capability id={} was written into the buffer", capabilityId));
        return true;
    }

    // capabilities::IContext
    bool CapabilitiesDelegatesManager::setCapabilityAvailability(
        const capabilities::TId capabilityId,
        const bool available) noexcept {
        const auto [iter, found] = find(m_capabilityData, capabilityId);
        if(!found) {
            m_log.log(Severity::Error, fmt::format("Failed to find capability data for id={}", capabilityId));
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
            m_log.log(Severity::Error, fmt::format("Failed to find capability data for id={}", capabilityId));
            return std::nullopt;
        }

        return iter->second.m_parameters;
    }
    
    std::optional<capabilities::Parameter> CapabilitiesDelegatesManager::getParamForCapabilityForKey(
        const capabilities::TId capabilityId,
        const capabilities::TKey& key) const noexcept {
        const auto [iterId, foundId] = find(m_capabilityData, capabilityId);
        if(!foundId) {
            m_log.log(Severity::Error, fmt::format("Failed to find capability data for id={}", capabilityId));
            return std::nullopt;
        }

        const auto [iterKey, foundKey] = find(iterId->second.m_parameters, key);
        if(!foundKey) {
            m_log.log(Severity::Error, fmt::format("Failed to find parameter for id={} and key={}", capabilityId, key));
            return std::nullopt;
        }

        return iterKey->second;
    }

    capabilities::IContext::TCapabilityIdToDataMap CapabilitiesDelegatesManager::getCapabilityIdToDataMap() const {
        return m_capabilityData;
    }
}
