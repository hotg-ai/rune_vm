//
// Created by Kirill Delimbetov - github.com/delimbetov - on 06.04.2021.
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <cmath>
#include <random>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <gtest/gtest.h>
#include <rune_vm/RuneVm.hpp>
#include "Common.hpp"
#include "TestLogger.hpp"

using namespace rune_vm;

struct RandDelegate : public CommonTestDelegate<capabilities::Capability::Rand> {
    RandDelegate(const float input)
        : CommonTestDelegate([input] {
            auto data = std::vector<uint8_t>();

            data.resize(sizeof(input));
            std::memcpy(data.data(), &input, sizeof(input));

            return data;
        }()) {}
};

struct SineRuneTest
    : public testing::TestWithParam<
        std::tuple<WasmBackend, TThreadCount, std::optional<uint32_t>, std::optional<uint32_t>, size_t>> {

    auto loadAndRun(float& output, const std::vector<capabilities::IDelegate::Ptr>& delegates) {
        const auto [backend, threadCount, optStackSizeBytes, optMemoryLimit, iterationCount] = GetParam();
        const auto logger = std::make_shared<const TestLogger>();
        logger->log(
            Severity::Info,
            "SineRuneTests.cpp",
            fmt::format(
                "Test params: tcount={} stackSizeBytes={} memory limit={} iterationCount={}",
                threadCount,
                optStackSizeBytes.value_or(0),
                optMemoryLimit.value_or(0),
                iterationCount));
        auto engine = createEngine(logger, backend, threadCount);
        auto runtime = engine->createRuntime(optStackSizeBytes, optMemoryLimit);
        auto rune = runtime->loadRune(delegates, g_testDataDirectory + "/sine/sine.rune");
        ASSERT_TRUE(rune);
        for(auto& delegate: delegates)
            static_cast<RandDelegate&>(*delegate).setRuneId(rune->id());

        // sine rune outputs [float]
        // assert top-level result is array
        for(auto iteration = 0ul; iteration < iterationCount; ++iteration) {
            auto result = rune->call();
            ASSERT_TRUE(result);
            ASSERT_EQ(result->count(), 1);
            ASSERT_EQ(result->typeAt(0), rune_vm::IResult::Type::Json);

            const auto jsonResult = nlohmann::json::parse(result->asJson());

            // assert it was array with 1 float
            ASSERT_TRUE(jsonResult.is_array());
            const auto& firstElement = jsonResult[0];
            ASSERT_TRUE(firstElement.is_number_float());
            const auto& elementResult = firstElement.get<float>();

            output = elementResult;
            ASSERT_LE(output, 1.f);
            ASSERT_GE(output, -1.f);
        }
    }

    void testCustomWithInput(
        const float input,
        const float expected,
        const float acceptableDiff) {
        const auto delegate = std::make_shared<RandDelegate>(input);
        auto output = 100.f;

        loadAndRun(output, {delegate});
        // sine rune is inexact
        ASSERT_NEAR(output, expected, acceptableDiff);
    }

    // setup
    void SetUp() override {
        std::random_device rd;
        m_engine = std::mt19937(rd());
        m_distrib = std::uniform_real_distribution<>(0.f, 2 * M_PI);
    }

    std::mt19937 m_engine;
    std::uniform_real_distribution<> m_distrib;
};

TEST_P(SineRuneTest, DefaultRandDelegate) {
    auto output = 100.f;
    loadAndRun(output, {});
}

TEST_P(SineRuneTest, CustomRandDelegate) {
    const auto input = m_distrib(m_engine);

    // sine rune is inexact
    testCustomWithInput(input, std::sin(input), 0.1f);
}

TEST_P(SineRuneTest, CustomRandDelegateKnownValue) {
    constexpr auto input = 0.8f;

    testCustomWithInput(input, 6.972786e-1, 0.0001f);
}

INSTANTIATE_TEST_SUITE_P(
    CorrectArguments,
    SineRuneTest,
    testing::Combine(
        testing::ValuesIn(g_wasmBackends),
        testing::ValuesIn(g_threadCounts),
        testing::Values(std::nullopt, 1 << 15, 1 << 20, uint32_t(-1)),
        testing::Values(std::nullopt, 1 << 25, uint32_t(-1)),
        testing::Values(1, 10)));
