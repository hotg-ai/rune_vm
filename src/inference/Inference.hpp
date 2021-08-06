//
// Created by Kirill Delimbetov - github.com/delimbetov - on 31.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <rune_vm/RuneVm.hpp>

namespace rune_vm_internal::inference {
    enum class InferenceBackend: uint8_t {
        TfLite
    };

    struct IVisitor;

    template<typename TBase, typename TVisitor>
    struct IElement: TBase {
        virtual void accept(TVisitor& visitor) noexcept = 0;
    };

    struct IModel: IElement<rune_vm::VirtualInterface<IModel>, IVisitor> {
        std::vector<rune_vm::DataView<const uint8_t>> inputs;
        std::vector<rune_vm::DataView<uint8_t>> outputs;

        static size_t tensorSize(const std::string &descriptorString);
    };

    struct IRuntime: rune_vm::VirtualInterface<IRuntime> {
        struct Options {
            rune_vm::TThreadCount m_numThreads;
        };

        virtual bool run(
            const IModel::Ptr& model,
            const rune_vm::DataView<const rune_vm::DataView<const uint8_t>> inputs,
            const rune_vm::DataView<rune_vm::DataView<uint8_t>> outputs) noexcept = 0;
        virtual IModel::Ptr loadModel(
            const rune_vm::DataView<const uint8_t> model,
            const uint32_t inputs,
            const uint32_t outputs) = 0;
    };

    [[nodiscard]] IRuntime::Ptr createRuntime(
        const rune_vm::ILogger::CPtr& logger,
        const InferenceBackend backend,
        const IRuntime::Options& options);
}
