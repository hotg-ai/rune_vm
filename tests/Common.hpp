//
// Created by Kirill Delimbetov - github.com/delimbetov - on 07.04.2021.
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <array>
#include <rune_vm/RuneVm.hpp>

constexpr auto g_wasmBackends = std::array{rune_vm::WasmBackend::Wasm3};
constexpr rune_vm::TThreadCount g_threadCounts[] = {1, 2, 4, 8, 16, 32};
const auto g_testDataDirectory = std::string(TEST_DATA_DIRECTORY);
