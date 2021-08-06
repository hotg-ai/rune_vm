//
// Created by Kirill Delimbetov - github.com/delimbetov - on 31.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <fmt/format.h>
#include <Common.hpp>
#include <inference/Inference.hpp>
#include <inference/tflite/TfLiteRuntime.hpp>
#include <numeric>
#include <regex>

namespace rune_vm_internal::inference {
    using namespace rune_vm;

    IRuntime::Ptr createRuntime(
        const ILogger::CPtr& logger,
        const InferenceBackend backend,
        const IRuntime::Options& options) {
        CHECK_THROW(logger);
        switch(backend) {
            case InferenceBackend::TfLite:
                return std::make_shared<TfLiteRuntime>(logger, options);
            default:
                logger->log(
                    Severity::Error,
                    "Inference.cpp",
                    fmt::format("Unknown backend was requested: {}", backend));
                CHECK_THROW(false);
        }
    }

    size_t IModel::tensorSize(const std::string &descriptorString) {
        size_t totalSize = 0;

        try {
            std::regex componentMatcher("(.*)\\[(.*)\\]");
            std::smatch componentMatches;
            std::regex_search(descriptorString, componentMatches, componentMatcher);

            if (componentMatches.size() == 3) {
                const auto &d = componentMatches[1];
                const std::string shapeString = componentMatches[2];

                // FIXME: implement for datatype = string
                int b = (d == "c128")                                   ? 16:
                        (d == "i64" || d == "c64" || d == "f64")        ? 8 :
                        (d == "f32" || d == "i32")                      ? 4 :
                        (d == "i16" || d == "f16")                      ? 2 :
                        (d == "b" || d == "u8" || d  == "i8")           ? 1 :
                                                                          0 ;

                totalSize = b;
                std::regex numberMatcher("(\\d+)");
                for (std::sregex_iterator it(shapeString.cbegin(), shapeString.cend(), numberMatcher), end; it != end; ++it) {
                    totalSize *= std::stoi(it->str());
                }

            }
        }  catch (const std::exception &e) {
            // Do nothing
        }

        return  totalSize;
    }

}
