//
// Created by Kirill Delimbetov - github.com/delimbetov - on 28.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <rune_vm/Log.hpp>

struct M3Runtime;

namespace rune_vm_internal {
    void checkM3Error(const rune_vm::LoggingModule& log, const std::shared_ptr<M3Runtime>& runtime, const char* result);

    template<typename TFunc, typename ...Args>
    void checkedCall(
        const rune_vm::LoggingModule& log,
        const std::shared_ptr<M3Runtime>& runtime,
        TFunc func,
        Args&&... args) {
        const auto result = func(std::forward<Args>(args)...);

        checkM3Error(log, runtime, result);
    }
}


