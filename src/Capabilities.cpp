//
// Created by Kirill Delimbetov - github.com/delimbetov - on 17.04.2021.
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <rune_vm/Capabilities.hpp>
#include <Common.hpp>

namespace rune_vm::capabilities {
    IContext::CapabilityData::CapabilityData(IDelegate::Ptr owner, const Capability capability)
        : m_owner(std::move(owner))
        , m_capability(capability)
        , m_availability(true) {
        CHECK_THROW(m_owner);
    }

    IDelegate::Ptr IContext::CapabilityData::owner() const noexcept {
        return m_owner;
    }
}