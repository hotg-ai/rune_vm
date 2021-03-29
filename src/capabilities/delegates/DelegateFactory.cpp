//
// Created by Kirill Delimbetov - github.com/delimbetov - on 29.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <capabilities/CapabilitiesDelegatesManager.hpp>
#include <capabilities/delegates/RandomCapabilityDelegate.hpp>

namespace rune_vm_internal {
    std::vector<TDelegateFactory> g_defaultDelegateFactories{{
        [](const rune_vm::ILogger::CPtr& logger) { return std::make_shared<RandomCapabilityDelegate>(logger); }
    }};
}