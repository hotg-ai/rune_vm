//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <rune_vm/RuneVm.hpp>
#include <HostFunctions.hpp>

struct M3Module;
struct M3Runtime;

namespace rune_vm_internal {
    class Wasm3Rune: public rune_vm::IRune, private host_functions::IHostContext {
    public:
        Wasm3Rune(
            const rune_vm::ILogger::CPtr& logger,
            std::shared_ptr<M3Module> module,
            std::shared_ptr<M3Runtime> runtime);

    private:
        // IRune
        void attachObserver(rune_vm::IRuneResultObserver::Ptr observer) final;
        void detachObserver(rune_vm::IRuneResultObserver::Ptr observer) final;

        // calls without waiting for observer call
        void call() final;
        // calls and waits for the observer to be called
        void callWaitResult(const std::chrono::microseconds) final;

        // internal
        template<auto functionName>
        void link();

        // data
        rune_vm::LoggingModule m_log;
        std::shared_ptr<M3Module> m_module;
        std::shared_ptr<M3Runtime> m_runtime;
    };
}


