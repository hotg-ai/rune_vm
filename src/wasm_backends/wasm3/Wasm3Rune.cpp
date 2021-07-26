//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <array>
#include <type_traits>
#include <fmt/format.h>
#include <m3_api_defs.h>
#include <m3_api_wasi.h>
#include <m3_env.h>
#include <wasm3.h>
#include <Common.hpp>
#include <HostFunctions.hpp>
#include <wasm_backends/wasm3/Wasm3Common.hpp>
#include <wasm_backends/wasm3/Wasm3Rune.hpp>

using namespace rune_vm_internal;

namespace {
    namespace wasm3_interop {
        // copied from wasm3 mostly
        using TStackType = uint64_t*;
        using TMemType = void*;

        void checkMemoryThrow(void* memory, uint64_t size, IM3Runtime runtime, TMemType _mem) {
            const auto res = [memory, size, runtime, _mem] {
                // TODO: m3ApiCheckMem doesnt work. why?
//                m3ApiCheckMem(memory, size);
                return m3Err_none; }();
            CHECK_THROW(res == m3Err_none);
        }

        template<typename T>
        void checkMemoryThrow(const TStackType stack, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow(stack, sizeof(T), runtime, _mem);
        }

        template<typename T>
        void checkMemoryThrow(const TStackType stack, rune_vm::DataView<T, uint32_t> dest, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow(stack, 2 * sizeof(uint32_t), runtime, _mem);
        }

        template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
        void arg_from_stack(T &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<rune_interop::TIntType>(psp, runtime, _mem);
            dest = rune_interop::fromInt<T>(*(rune_interop::TIntType *) (psp));
            psp++;
        }

        template<typename T, std::enable_if_t<!std::is_enum_v<T>, int> = 0>
        void arg_from_stack(T &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<T>(psp, runtime, _mem);
            dest = *(T *) (psp);
            psp++;
        }

        template<typename T, typename TSize>
        void arg_from_stack(rune_vm::DataView<T, TSize>& dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow(psp, dest, runtime, _mem);
            dest.m_data = (T*) m3ApiOffsetToPtr(* ((u32 *) (psp++)));
            dest.m_size = *(TSize *) (psp++);
        }

        template<typename T, typename TSize>
        void arg_from_stack(rune_vm::DoubleNestedDataView<T, TSize>& dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
//            checkMemoryThrow(psp, dest, runtime, _mem);
            dest.m_data = (rune_vm::DataView<T, TSize>*) m3ApiOffsetToPtr(* ((u32 *) (psp++)));
            dest.m_size = *(TSize *) (psp++);

            for (size_t i = 0; i < dest.m_size; i++) {
                dest.m_data[i].m_data = (T*) m3ApiOffsetToPtr((size_t)(dest.m_data[i].m_data));
            }
        }

        template<typename T>
        void arg_from_stack(T* &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<T*>(psp, runtime, _mem);
            dest = (void*) m3ApiOffsetToPtr(* ((u32 *) (psp++)));
        };

        template<typename T>
        void arg_from_stack(const T* &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<T*>(psp, runtime, _mem);
            dest = (void*) m3ApiOffsetToPtr(* ((u32 *) (psp++)));
        };

        template<typename T>
        void arg_from_stack(T** &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<T**>(psp, runtime, _mem);
            dest = (uint8_t**) m3ApiOffsetToPtr(* ((u32 *) (psp++)));
        };

        template<typename T>
        void arg_from_stack(const T** &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<T**>(psp, runtime, _mem);
            dest = (const uint8_t**) m3ApiOffsetToPtr(* ((u32 *) (psp++)));
        };

        template <typename ...Args>
        static void get_args_from_stack(TStackType &sp, IM3Runtime runtime, TMemType mem, std::tuple<Args...> &tuple) {
            std::apply([&](auto &... item) {
                (arg_from_stack(item, sp, runtime, mem), ...);
            }, tuple);
        }

        template<auto func>
        struct wrap_helper;

        template <typename Ret, typename ...Args, Ret (*Fn)(Args...)>
        struct wrap_helper<Fn> {
            static const void *wrap_fn(IM3Runtime rt, IM3ImportContext _ctx, TStackType sp, TMemType mem) {
                Ret *ret_ptr = (Ret *) (sp);
                std::tuple<Args...> args;
                try {
                    get_args_from_stack(sp, rt, mem, args);
                } catch(...) {
                    // can't properly log it here
                    static_assert(std::is_same_v<Ret, std::underlying_type_t<rune_interop::ReturnCode>>);
                    *ret_ptr = rune_interop::RC_InputError;
                    return m3Err_malformedFunctionSignature;
                }
                Ret r = std::apply(Fn, args);
                *ret_ptr = r;
                return m3Err_none;
            }
        };

        template <typename Ret, typename ...Args, Ret (*Fn)(host_functions::HostContext*, Args...)>
        struct wrap_helper<Fn> {
            static const void *wrap_fn(IM3Runtime rt, IM3ImportContext _ctx, TStackType sp, TMemType mem) {
                Ret *ret_ptr = (Ret *) (sp);
                auto* context = reinterpret_cast<host_functions::HostContext*>(_ctx->userdata);
                std::tuple<Args...> args;
                try {
                    CHECK_THROW(context);
                    get_args_from_stack(sp, rt, mem, args);
                } catch(...) {
                    // can't properly log it here
                    static_assert(std::is_same_v<Ret, std::underlying_type_t<rune_interop::ReturnCode>>);
                    *ret_ptr = rune_interop::RC_InputError;
                    return m3Err_malformedFunctionSignature;
                }
                Ret r = std::apply(Fn, std::tuple_cat(std::make_tuple(context), std::move(args)));
                *ret_ptr = r;
                return m3Err_none;
            }
        };

        template <typename ...Args, void (*Fn)(Args...)>
        struct wrap_helper<Fn> {
            static const void *wrap_fn(IM3Runtime rt, IM3ImportContext _ctx, TStackType sp, TMemType mem) {
                std::tuple<Args...> args;
                try {
                    get_args_from_stack(sp, rt, mem, args);
                } catch(...) {
                    return m3Err_malformedFunctionSignature;
                }
                std::apply(Fn, args);
                return m3Err_none;
            }
        };

        template<typename... Arrs>
        constexpr auto concat(const Arrs&... arrs) {
            std::size_t cur = 0;
            std::array<std::common_type_t<typename Arrs::value_type...>, (std::tuple_size_v<Arrs> + ...)> result{};
            ([&](const auto& arr) {
                for (const auto& elem : arr) {
                    result[cur++] = elem;
                }
            }(arrs), ...);

            return result;
        }

        template<char ...c>
        struct m3_sig {
            static constexpr std::array<char, sizeof...(c)> value = {c...};
        };

        template<typename T, typename T2 = void> struct m3_type_to_sig;
        template<typename T> struct m3_type_to_sig<T, std::enable_if_t<std::is_enum_v<T>>> : m3_sig<'i'> {};
        template<> struct m3_type_to_sig<i32> : m3_sig<'i'> {};
        template<> struct m3_type_to_sig<u32> : m3_sig<'i'> {};
        template<> struct m3_type_to_sig<i64> : m3_sig<'I'> {};
        template<> struct m3_type_to_sig<f32> : m3_sig<'f'> {};
        template<> struct m3_type_to_sig<f64> : m3_sig<'F'> {};
        template<> struct m3_type_to_sig<void> : m3_sig<'v'> {};
        template<> struct m3_type_to_sig<void *> : m3_sig<'*'> {};
        template<> struct m3_type_to_sig<const void *> : m3_sig<'*'> {};
        template<> struct m3_type_to_sig<uint8_t **> : m3_sig<'*'> {};
        template<> struct m3_type_to_sig<const uint8_t **> : m3_sig<'*'> {};
        template<typename T, typename TSize> struct m3_type_to_sig<rune_vm::DataView<T, TSize>> : m3_sig<'*', 'i'> {};
        template<typename T, typename TSize> struct m3_type_to_sig<rune_vm::DoubleNestedDataView<T, TSize>> : m3_sig<'*', 'i'> {};

        template<typename Ret, typename ... Args>
        struct m3_signature {
            constexpr static size_t n_args = sizeof...(Args);
            constexpr static auto value = concat(
                m3_type_to_sig<Ret>::value,
                m3_sig<'('>::value,
                m3_type_to_sig<Args>::value...,
                m3_sig<')'>::value,
                m3_sig<'\0'>::value
            );
        };

        template<typename Ret, typename ...Args>
        constexpr auto getSignature(Ret (*)(Args...)) {
            return m3_signature<Ret, Args...>::value;
        }

        template<typename Ret, typename ...Args>
        constexpr auto getSignature(Ret (*)(host_functions::HostContext*, Args...)) {
            return m3_signature<Ret, Args...>::value;
        }
    }
}

//TODO: Turn this into a proper function
#define LINK(functionName, function) {                                                      \
    constexpr auto signature = wasm3_interop::getSignature(function);                       \
    m_log.log(Severity::Info,                                                               \
              fmt::format("Linking to function name={} signature={}",                       \
                          functionName, signature.data()));                                 \
    const auto result = m3_LinkRawFunctionEx(m_module.get(),                                \
                                             rune_interop::g_moduleName,                    \
                                             functionName,                                  \
                                             signature.data(),                              \
                                             wasm3_interop::wrap_helper<function>::wrap_fn, \
                                             &m_hostContext);                               \
    if (result != m3Err_functionLookupFailed) checkM3Error(m_log, m_runtime, result);       \
}

namespace rune_vm_internal {
    using namespace rune_vm;

    Wasm3Rune::Wasm3Rune(
        const rune_vm::ILogger::CPtr& logger,
        const rune_vm::TRuneId runeId,
        std::shared_ptr<M3Module> module,
        std::shared_ptr<M3Runtime> runtime,
        const std::vector<rune_vm::capabilities::IDelegate::Ptr>& delegates,
        const inference::ModelManager::Ptr& modelManager)
        : m_log(logger, fmt::format("Wasm3Rune id={}", runeId))
        , m_module(std::move(module))
        , m_runtime(std::move(runtime))
        , m_hostContext(
            logger,
            runeId,
            std::make_shared<CapabilitiesDelegatesManager>(logger, delegates),
            modelManager)
        , m_callFunction(nullptr) {
        m_log.log(Severity::Debug, fmt::format("Wasm3Rune() id={}", runeId));

        // Link host functions
        using namespace rune_interop::host_function_rune_name;
        LINK(g_requestCapability, rune_vm_internal::host_functions::requestCapability);
        LINK(g_requestCapabilitySetParam, rune_vm_internal::host_functions::requestCapabilitySetParam);
        LINK(g_requestProviderResponse, rune_vm_internal::host_functions::requestProviderResponse);
        LINK(g_tfmPreloadModel, rune_vm_internal::host_functions::tfmPreloadModel);
        LINK(g_tfmModelInvoke, rune_vm_internal::host_functions::tfmModelInvoke);
        LINK(g_requestOutput, rune_vm_internal::host_functions::requestOutput);
        LINK(g_consumeOutput, rune_vm_internal::host_functions::consumeOutput);
        LINK(g_debug, rune_vm_internal::host_functions::debug);
        LINK(g_runeModelLoad, rune_vm_internal::host_functions::runeModelLoad);
        LINK(g_runeModelInfer, rune_vm_internal::host_functions::runeModelInfer);

        // Lookup manifest function
        auto manifestFunction = IM3Function();

        checkedCall(
            m_log,
            m_runtime,
            m3_FindFunction,
            &manifestFunction,
            m_runtime.get(),
            rune_interop::rune_function_name::g_manifest);
        CHECK_THROW(manifestFunction);

        // Lookup call function
        checkedCall(
            m_log,
            m_runtime,
            m3_FindFunction,
            &m_callFunction,
            m_runtime.get(),
            rune_interop::rune_function_name::g_call);
        CHECK_THROW(m_callFunction);

        // Call manifest function
        checkedCall(
            m_log,
            m_runtime,
            m3_CallV,
            manifestFunction);
    }

    // IRune
    TRuneId Wasm3Rune::id() const noexcept {
        return m_hostContext.runeId();
    }

    capabilities::IContext::Ptr Wasm3Rune::getCapabilitiesContext() const noexcept {
        return m_hostContext.capabilitiesManager();
    }

    IResult::Ptr Wasm3Rune::call() {
        m_log.log(Severity::Debug, "call()");
        // call accepts 3 arguments which are ignored - see last 3 zeros
        checkedCall(
            m_log,
            m_runtime,
            m3_CallV,
            m_callFunction,
            0,
            0,
            0);

        // TODO: rethink output manager
        const auto optOutputId = m_hostContext.outputManager().lastSavedId();
        CHECK_THROW(optOutputId);
        const auto optResult = m_hostContext.outputManager().consumeOutput(*optOutputId);
        CHECK_THROW(optResult);

        return *optResult;
    }
}
