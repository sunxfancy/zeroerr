#pragma once

#include "zeroerr/internal/config.h"
#include "zeroerr/unittest.h"
#include "zeroerr/domains/arbitrary.h"

#include <functional>
#include <string>
#include <vector>
#include <type_traits>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

#define ZEROERR_CREATE_FUZZ_TEST_FUNC(function, name)                                     \
    static void                     function(zeroerr::TestContext*);                      \
    static zeroerr::detail::regTest ZEROERR_NAMEGEN(_zeroerr_reg)(                        \
        {name, __FILE__, __LINE__, function}, zeroerr::TestType::fuzz_test); \
    static void function(ZEROERR_UNUSED(zeroerr::TestContext* _ZEROERR_TEST_CONTEXT))

#define FUZZ_TEST_CASE(name) ZEROERR_CREATE_FUZZ_TEST_FUNC(ZEROERR_NAMEGEN(_zeroerr_testcase), name)

#define FUZZ_FUNC(func) zeroerr::FuzzFunction(func, _ZEROERR_TEST_CONTEXT)


namespace zeroerr {

namespace detail {
template <typename... Args>
struct FunctionDecomposition {
    static constexpr size_t numOfArgs = sizeof...(Args);
    using SeedType = std::tuple<Args...>;
};

template <typename... Args>
FunctionDecomposition<typename std::decay<Args>::type...> FunctionDecompositionImpl
(void (*)(Args...));

template <typename... Args>
FunctionDecomposition<typename std::decay<Args>::type...> FunctionDecompositionImpl
(std::function<void(Args...)>);


template<typename T>
struct memfun_type
{
    using type = void;
};

template<typename Ret, typename Class, typename... Args>
struct memfun_type<Ret (Class::*)(Args...) const>
{
    using ret_type = FunctionDecomposition<typename std::decay<Args>::type...>;
};

template <typename F>
typename memfun_type<decltype(&F::operator())>::ret_type FunctionDecompositionImpl(F);


template <typename T>
using FunctionDecompositionT = decltype(FunctionDecompositionImpl(std::declval<T>()));

}  // namespace detail

template <typename TargetFunction,
          typename Base = detail::FunctionDecompositionT<TargetFunction>>
struct FuzzTest
    : public Base {
    using SeedType = typename Base::SeedType;

    template <typename... T>
    FuzzTest& WithDomains(T&&... domains) {

        return *this;
    }

    FuzzTest& WithSeeds(std::initializer_list<SeedType>&& seeds) {

        return *this;
    }

    void Run(int count = 1000, int seed = 0) {

    }
};

template <typename T>
FuzzTest<T> FuzzFunction(T func, TestContext* context) {
    
    return FuzzTest<T>();
}

template <typename T>
std::vector<T> ReadCorpusFromDir(std::string dir);


}  // namespace zeroerr


ZEROERR_SUPPRESS_COMMON_WARNINGS_POP