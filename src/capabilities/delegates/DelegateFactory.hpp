//
// Created by Kirill Delimbetov - github.com/delimbetov - on 29.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <rune_vm/Capabilities.hpp>
#include <rune_vm/Log.hpp>

namespace rune_vm_internal {
    std::vector<rune_vm::capabilities::Capability> getAllSupportedByDefaultCapabilities();
    std::optional<rune_vm::capabilities::IDelegate::Ptr> getDefaultDelegateImplementingCapability(
        const rune_vm::ILogger::CPtr& logger,
        const rune_vm::capabilities::Capability capability);
}