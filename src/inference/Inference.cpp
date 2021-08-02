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

    size_t TensorDescriptor::byteCount() const
    {
        // FIXME: implement for datatype = string
        size_t bytesPerUnit = (dataType == DataType__NoType || dataType == DataType__Bool || dataType == DataType__UInt8 || dataType == DataType__Int8 ) ? 1:
                              (dataType == DataType__Int16 || dataType == DataType__Float16 )                                                            ? 2:
                              (dataType == DataType__Float32 || dataType == DataType__Int32 )                                                            ? 4:
                              (dataType == DataType__Int64 || dataType == DataType__Complex64 || dataType == DataType__Float64)                          ? 8:
                              (dataType == DataType__Complex128)                                                                                         ? 8:
                                                                                                                                                           0;

        return bytesPerUnit * std::accumulate(shape.begin(), shape.end(), decltype(shape)::value_type(1),  std::multiplies<int>());
    }

    TensorDescriptor::TensorDescriptor(const std::string &descriptorString)
    {
        try {
            std::regex componentMatcher("(.*)\\[(.*)\\]");
            std::smatch componentMatches;
            std::regex_search(descriptorString, componentMatches, componentMatcher);

            if (componentMatches.size() == 3) {
                const auto &dataTypeString = componentMatches[1];
                const std::string shapeString = componentMatches[2];

                if (dataTypeString == "f32") {
                    dataType = DataType__Float32;
                } else if (dataTypeString == "i32") {
                    dataType = DataType__Int32;
                } else if (dataTypeString == "u8") {
                    dataType = DataType__UInt8;
                } else if (dataTypeString == "i64") {
                    dataType = DataType__Int64;
                } else if (dataTypeString == "s") {
                    dataType = DataType__String;
                } else if (dataTypeString == "b") {
                    dataType = DataType__Bool;
                } else if (dataTypeString == "i16") {
                    dataType = DataType__Int16;
                } else if (dataTypeString == "c64") {
                    dataType = DataType__Complex64;
                } else if (dataTypeString == "i8") {
                    dataType = DataType__Int8;
                } else if (dataTypeString == "f16") {
                    dataType = DataType__Float16;
                } else if (dataTypeString == "f64") {
                    dataType = DataType__Float64;
                } else if (dataTypeString == "c128") {
                    dataType = DataType__Complex128;
                } else {
                    dataType = DataType__NoType;
                }

                std::regex numberMatcher("(\\d+)");
                for (std::sregex_iterator it(shapeString.cbegin(), shapeString.cend(), numberMatcher), end; it != end; ++it) {
                     shape.push_back(std::stoi(it->str()));
                }
            }
        }  catch (const std::exception &e) {
            dataType = DataType__NoType;
            shape.clear();
        }
    }
}
