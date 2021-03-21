//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <m3_env.h>
#include <wasm3.h>
#include <wasm3_cpp.h>
#include "Wasm3Rune.hpp"

namespace rune_vm_internal {
    using namespace rune_vm;

    Wasm3Rune::Wasm3Rune(const rune_vm::ILogger::CPtr& logger, std::shared_ptr<M3Module> module)
        : m_log(logger, "Wasm3Rune")
        , m_module(std::move(module)) {
        m_log.log(Severity::Debug, "Wasm3Rune()");

        // Link host functions
        // TODO: make host function independent of specific backend
        link(m_module.get(), "env", "tfm_preload_model", &m3_tfm_preload_model);
        link(m_module.get(), "env", "tfm_model_invoke", &m3_tfm_model_invoke);

        link(m_module.get(), "env", "request_capability", &m3_request_capability);
        link(m_module.get(), "env", "request_capability_set_param", &m3_request_capability_set_param);
        link(m_module.get(), "env", "request_provider_response", &m3_request_provider_response);

        link(m_module.get(), "env", "request_output", &m3_request_output);
        link(m_module.get(), "env", "consume_output", &m3_consume_output);
        link(m_module.get(), "env", "request_manifest_output", &m3_manifest_output);

        link(m_module.get(), "env", "_debug", &m3_debug);
    }

    // IRune
    void attachObserver(rune_vm::IRuneResultObserver::Ptr observer);
    void detachObserver(rune_vm::IRuneResultObserver::Ptr observer);

    // calls without waiting for observer call
    void call();
    // calls and waits for the observer to be called
    void callWaitResult(const std::chrono::microseconds);

    // Internal
    template<typename Ret, typename ... Args>
    void Wasm3Rune::link(const char* moduleName, const char* functionName, Ret (*function)(Args...)) {
        m3_LinkRawFunction(
            m_module.get(),
            moduleName,
            functionName,
            wasm3::detail::m3_signature<Ret, Args...>::value,
            function);
    }

}