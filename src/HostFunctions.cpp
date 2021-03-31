//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <Common.hpp>
#include <HostFunctions.hpp>

namespace rune_vm_internal::host_functions {
    using namespace rune_vm;

    HostContext::HostContext(
        const rune_vm::ILogger::CPtr& logger,
        CapabilitiesDelegatesManager::Ptr&& capabilitiesManager,
        const inference::IRuntime::Ptr& inferenceRuntime)
        : m_log(logger, "HostContext")
        , m_capabilitiesManager(std::move(capabilitiesManager))
        , m_inferenceRuntime(inferenceRuntime) {
        CHECK_THROW(m_capabilitiesManager);
        CHECK_THROW(m_inferenceRuntime);
        m_log.log(Severity::Debug, "HostContext()");
    }

    const rune_vm::LoggingModule& HostContext::log() const noexcept {
        return m_log;
    }

    const CapabilitiesDelegatesManager::Ptr& HostContext::capabilitiesManager() noexcept {
        return m_capabilitiesManager;
    }

    const CapabilitiesDelegatesManager::Ptr& HostContext::capabilitiesManager() const noexcept {
        return m_capabilitiesManager;
    }

    const inference::IRuntime::Ptr& HostContext::inferenceRuntime() noexcept {
        return m_inferenceRuntime;
    }

    //
    TCapabilityId requestCapability(HostContext* context, const rune_interop::Capability capabilityType) noexcept {

//        auto capability_idx = MANIFEST_->add_capability((hmr::CAPABILITY)capability_type);
//        if (LOGSTREAM.is_open()) {
//            LOGSTREAM << "Manifest::Capability request " << capability_type << std::endl;
    }

}