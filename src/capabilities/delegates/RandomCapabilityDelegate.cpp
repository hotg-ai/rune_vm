//
// Created by Kirill Delimbetov - github.com/delimbetov - on 29.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <cmath>
#include <io/FormatCapabilitiesParameter.hpp>
#include <Common.hpp>
#include <capabilities/delegates/RandomCapabilityDelegate.hpp>

namespace rune_vm_internal {
    using namespace rune_vm;

    RandomCapabilityDelegate::RandomCapabilityDelegate(const ILogger::CPtr& logger)
        : m_log(logger, "RandomCapabilityDelegate")
        , m_supportedCapabilities(g_supportedCapabilities.begin(), g_supportedCapabilities.end()) {
        m_log.log(Severity::Debug, "RandomCapabilityDelegate()");
    }

    // IDelegate
    RandomCapabilityDelegate::TCapabilitiesSet
        RandomCapabilityDelegate::getSupportedCapabilities() const noexcept {
        return m_supportedCapabilities;
    }
    
    bool RandomCapabilityDelegate::requestCapability(
        const TRuneId runeId,
        const capabilities::Capability capability,
        const capabilities::TId newCapabilityId) noexcept {
        m_log.log(
            Severity::Debug,
            fmt::format("requestCapability rune_id={} capability={} id={}", runeId, capability, newCapabilityId));
        const auto [iter, found] = find(m_engines, newCapabilityId);
        if(found) {
            m_log.log(
                Severity::Warning,
                fmt::format("Requesting capability for already allocated id={}", newCapabilityId));
            return false;
        }

        if(m_supportedCapabilities.count(capability) == 0) {
            m_log.log(
                Severity::Error,
                fmt::format("Requesting capability which is not supported={}", capability));
            return false;
        }

        try {
            m_engines.emplace(
                newCapabilityId,
                CapabilityIdData{.m_engine = std::mt19937(m_randomDevice()), .m_range = {0.f, 2.f * M_PI}});
            m_log.log(
                Severity::Debug,
                fmt::format("New capability={} allocated with id={}", capability, newCapabilityId));
        } catch(const std::exception& e) {
            m_log.log(Severity::Error, fmt::format("Failed to add engine for id={}: {}", newCapabilityId, e.what()));
            return false;
        }

        return true;
    }

    bool RandomCapabilityDelegate::requestCapabilityParamChange(
        const TRuneId runeId,
        const capabilities::TId capabilityId,
        const capabilities::TKey& key,
        const capabilities::Parameter& parameter) noexcept {
        m_log.log(
            Severity::Debug,
            fmt::format("requestCapabilityParamChange rune_id={} id={} key={} param={}", runeId, capabilityId, key, parameter));
        const auto [iter, found] = find(m_engines, capabilityId);
        if(!found) {
            m_log.log(
                Severity::Error,
                fmt::format("Trying to set parameter for non-allocated capability id={}", capabilityId));
            return false;
        }

        // NOTE: this is just an example implementation, no such param actually exist for Rand capability
        try {
            if(key == "low_bound")
                iter->second.m_range.first = std::get<float>(parameter.m_data);
            else if(key == "upp_bound")
                iter->second.m_range.second = std::get<float>(parameter.m_data);
            else
                CHECK_THROW(false);
        } catch(const std::bad_variant_access& e) {
            m_log.log(
                Severity::Error,
                fmt::format("Trying to set parameter of invalid type. Key={} expects float", key));
            return false;
        } catch(const std::exception& e) {
            m_log.log(
                Severity::Error,
                fmt::format("Failed to set parameter for key={}: {}", key, e.what()));
            return false;
        }

        return true;
    }

    bool RandomCapabilityDelegate::requestRuneInputFromCapability(
        const TRuneId runeId,
        const DataView<uint8_t> buffer,
        const capabilities::TId capabilityId) noexcept {
        m_log.log(
            Severity::Debug,
            fmt::format("requestRuneInputFromCapability rune_id={} id={} buffer bytes length={}", runeId, capabilityId, buffer.m_size));
        const auto [iter, found] = find(m_engines, capabilityId);
        if(!found) {
            m_log.log(
                Severity::Error,
                fmt::format("Trying to query input for non-allocated capability id={}", capabilityId));
            return false;
        }

        if(!buffer.m_data) {
            m_log.log(
                Severity::Error,
                fmt::format("Trying to query input for capability id={}, buffer nullptr", capabilityId));
            return false;
        }

        constexpr auto expectedSize = sizeof(float);
        if(buffer.m_size != expectedSize) {
            m_log.log(
                Severity::Error,
                fmt::format(
                    "Trying to query input for capability id={}, buffer size invalid: expected={}, actual={}",
                    capabilityId,
                    expectedSize,
                    buffer.m_size));
            return false;
        }

        const auto& range = iter->second.m_range;
        if(range.second <= range.first) {
            m_log.log(
                Severity::Error,
                fmt::format(
                    "Trying to query input for capability id={}, but range is invalid: [{}, {}]",
                    capabilityId,
                    range.first,
                    range.second));
            return false;
        }

        auto distribution = std::uniform_real_distribution(range.first, range.second);
        const auto randomFloat = float(distribution(iter->second.m_engine));

        std::memcpy(buffer.m_data, &randomFloat, sizeof(randomFloat));

        return true;
    }
}
