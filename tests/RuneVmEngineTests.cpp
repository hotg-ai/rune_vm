//
// Created by Kirill Delimbetov - github.com/delimbetov - on 06.04.2021.
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <gtest/gtest.h>
#include <rune_vm/RuneVm.hpp>
#include "Common.hpp"
#include "TestLogger.hpp"

using namespace rune_vm;

// Correct construction
struct RuneVmIEngineCorrectConstructorTest : public testing::TestWithParam<std::tuple<WasmBackend, TThreadCount>> {};

TEST_P(RuneVmIEngineCorrectConstructorTest, CorrectConstruction) {
    const auto [backend, threadCount] = GetParam();
    auto logger = std::make_shared<const TestLogger>();
    ASSERT_TRUE(createEngine(logger, backend, threadCount));
}

INSTANTIATE_TEST_SUITE_P(
    CorrectArguments,
    RuneVmIEngineCorrectConstructorTest,
    testing::Combine(testing::ValuesIn(g_wasmBackends), testing::ValuesIn(g_threadCounts)));

// Invalid construction
struct RuneVmIEngineInvalidConstructorTest : public testing::TestWithParam<std::tuple<WasmBackend, TThreadCount>> {};

TEST_P(RuneVmIEngineInvalidConstructorTest, InvalidConstruction) {
    const auto [backend, threadCount] = GetParam();
    auto logger = std::make_shared<const TestLogger>();
    ASSERT_THROW(createEngine(logger, backend, threadCount), std::runtime_error);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidArguments,
    RuneVmIEngineInvalidConstructorTest,
    testing::Combine(testing::ValuesIn(g_wasmBackends), testing::Values(0)));

TEST(RuneVmIEngineInvalidConstructorTest, NullLogger) {
    ASSERT_THROW(createEngine(nullptr, WasmBackend::Wasm3, 2), std::runtime_error);
}

// Valid runtime creation
struct RuneVmIEngineCreateRuntimeCorrectTest : public testing::TestWithParam<
        std::tuple<WasmBackend, TThreadCount, std::optional<uint32_t>, std::optional<uint32_t>>> {};

TEST_P(RuneVmIEngineCreateRuntimeCorrectTest, CorrectConstruction) {
    const auto [backend, threadCount, optStackSizeBytes, optMemoryLimit] = GetParam();
    auto logger = std::make_shared<const TestLogger>();
    const auto engine = createEngine(logger, backend, threadCount);
    ASSERT_TRUE(engine->createRuntime(optStackSizeBytes, optMemoryLimit));
}

INSTANTIATE_TEST_SUITE_P(
    CorrectArguments,
    RuneVmIEngineCreateRuntimeCorrectTest,
    testing::Combine(
        testing::ValuesIn(g_wasmBackends),
        testing::ValuesIn(g_threadCounts),
        testing::Values(std::nullopt, 1, 1 << 5, 1 << 10, 1 << 15, 1 << 20, uint32_t(-1)),
        testing::Values(std::nullopt, 1, 1 << 5, 1 << 10, 1 << 15, 1 << 20, uint32_t(-1))));

// Invalid runtime creation
// No arguments are invalid currently : )