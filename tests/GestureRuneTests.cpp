//
// Created by Kirill Delimbetov - github.com/delimbetov - on 08.04.2021.
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <cmath>
#include <random>
#include <gtest/gtest.h>
#include <csv.h>
#include <rune_vm/RuneVm.hpp>
#include "Common.hpp"
#include "TestLogger.hpp"

using namespace rune_vm;

const auto g_gestureRuneFilePath = g_testDataDirectory + "/gesture/gesture.rune";
const auto g_gestureExamples = std::vector<std::pair<std::string, std::string>>{
    {g_testDataDirectory + "/gesture/example_ring.csv", "Ring"},
    // Slope and wing are expected to fail - examples are outdated
    // TODO: update
    {g_testDataDirectory + "/gesture/example_slope.csv", "Slope"},
    {g_testDataDirectory + "/gesture/example_wing.csv", "Wing"},
};

struct AccelDelegate : public CommonTestDelegate<capabilities::Capability::Accel> {
    AccelDelegate(const std::vector<float>& input)
        : CommonTestDelegate([&input] {
            auto data = std::vector<uint8_t>();

            data.resize(input.size() * sizeof(float));
            std::memcpy(data.data(), input.data(), data.size());

            return data;
        }()) {}
};


struct GestureRuneTest
    : public testing::TestWithParam<
        std::tuple<WasmBackend, TThreadCount, std::pair<std::string, std::string>, std::optional<uint32_t>, std::optional<uint32_t>>> {
};

auto parseCsv(const std::string& path) {
    auto reader = io::CSVReader<3>(path);
    auto data = std::vector<float>();
    auto lineData = std::array<float, 3>();

    data.reserve(3 * 128);

    while(reader.read_row(lineData[0], lineData[1], lineData[2])) {
        data.push_back(lineData[0]);
        data.push_back(lineData[1]);
        data.push_back(lineData[2]);
    }

     return data;
}

TEST_P(GestureRuneTest, CustomDelegateKnownValue) {
    const auto& [backend, threadCount, example, optStackSizeBytes, optMemoryLimit] = GetParam();
    const auto& [examplePath, exampleExpectedOutput] = example;
    const auto input = parseCsv(examplePath);
    const auto logger = std::make_shared<const TestLogger>();
    auto engine = createEngine(logger, backend, threadCount);
    auto runtime = engine->createRuntime(optStackSizeBytes, optMemoryLimit);
    auto rune = runtime->loadRune({std::make_shared<AccelDelegate>(input)}, g_gestureRuneFilePath);
    ASSERT_TRUE(rune);

    // gesture rune outputs string
    auto result = rune->call();
    ASSERT_TRUE(result);
    ASSERT_EQ(result->count(), 1);
    ASSERT_EQ(result->typeAt(0), rune_vm::IResult::Type::String);
    const auto firstElement = result->getAt(0);
    ASSERT_TRUE(std::holds_alternative<std::string_view>(firstElement));
    const auto& elementResult = std::get<std::string_view>(firstElement);

    ASSERT_EQ(elementResult, exampleExpectedOutput);
}

INSTANTIATE_TEST_SUITE_P(
    CorrectArguments,
    GestureRuneTest,
    testing::Combine(
        testing::ValuesIn(g_wasmBackends),
        testing::ValuesIn(g_threadCounts),
        testing::ValuesIn(g_gestureExamples),
        testing::Values(std::nullopt, 1 << 15, 1 << 20, uint32_t(-1)),
        testing::Values(std::nullopt, 1 << 25, uint32_t(-1))));
