//
// Created by Kirill Delimbetov - github.com/delimbetov - on 28.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

// wasm3
#include <m3_env.h>
#include <wasm3.h>
//
#include <fmt/format.h>
#include <rune_vm/Log.hpp>
#include <Common.hpp>
#include <wasm_backends/wasm3/Wasm3Common.hpp>

namespace rune_vm_internal {
    void checkM3Error(
        const rune_vm::LoggingModule& log,
        const std::shared_ptr<M3Runtime>& runtime,
        const M3Result result) {
        if(result != m3Err_none) {
            auto info = M3ErrorInfo();

            m3_GetErrorInfo(runtime.get(), &info);
            CHECK_THROW(info.file && info.message);
            log.log(
        rune_vm::Severity::Error,
        fmt::format("M3 function has failed: file={} line={} msg={}", info.file, info.line, info.message));
            CHECK_THROW(false);
        }
    }
}