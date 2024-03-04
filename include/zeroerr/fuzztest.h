#pragma once

#include "zeroerr/internal/config.h"
#include "zeroerr/unittest.h"

#include "zeroerr/domains/aggregate_of.h"
#include "zeroerr/domains/arbitrary.h"
#include "zeroerr/domains/container_of.h"
#include "zeroerr/domains/element_of.h"
#include "zeroerr/domains/in_range.h"
#include "zeroerr/internal/rng.h"


#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

#define ZEROERR_CREATE_FUZZ_TEST_FUNC(function, name)                        \
    static void                     function(zeroerr::TestContext*);         \
    static zeroerr::detail::regTest ZEROERR_NAMEGEN(_zeroerr_reg)(           \
        {name, __FILE__, __LINE__, function}, zeroerr::TestType::fuzz_test); \
    static void function(ZEROERR_UNUSED(zeroerr::TestContext* _ZEROERR_TEST_CONTEXT))

#define FUZZ_TEST_CASE(name) ZEROERR_CREATE_FUZZ_TEST_FUNC(ZEROERR_NAMEGEN(_zeroerr_testcase), name)

#define FUZZ_FUNC(func) zeroerr::FuzzFunction(func, _ZEROERR_TEST_CONTEXT)


namespace zeroerr {

namespace detail {
template <typename... Args>
struct FunctionDecomposition {
    static constexpr size_t numOfArgs = sizeof...(Args);
    using ValueType                   = std::tuple<Args...>;
};

template <typename... Args>
FunctionDecomposition<typename std::decay<Args>::type...> FunctionDecompositionImpl(
    void (*)(Args...));

template <typename... Args>
FunctionDecomposition<typename std::decay<Args>::type...> FunctionDecompositionImpl(
    std::function<void(Args...)>);


template <typename T>
struct memfun_type {
    using type = void;
};

template <typename Ret, typename Class, typename... Args>
struct memfun_type<Ret (Class::*)(Args...) const> {
    using ret_type = FunctionDecomposition<typename std::decay<Args>::type...>;
};

template <typename F>
typename memfun_type<decltype(&F::operator())>::ret_type FunctionDecompositionImpl(F);


template <typename T>
using FunctionDecompositionT = decltype(FunctionDecompositionImpl(std::declval<T>()));

}  // namespace detail

template <typename TargetFunction,
          typename FuncType = detail::FunctionDecompositionT<TargetFunction>>
struct FuzzTest;

template <typename TargetFunction, typename FuncType, typename Domain,
          typename Base = FuzzTest<TargetFunction, FuncType>>
struct FuzzTestWithDomain;

struct IFuzzTest {
    virtual void        Run(int count = 1000, int seed = 0)          = 0;
    virtual void        RunOneTime(const uint8_t* data, size_t size) = 0;
    virtual std::string MutateData(const uint8_t* data, size_t size, size_t max_size,
                                   unsigned int seed)                = 0;
};

template <typename TargetFunction, typename FuncType>
struct FuzzTest : IFuzzTest {
    using ValueType = typename FuncType::ValueType;
    FuzzTest(TargetFunction func) : func(func) {}
    FuzzTest(const FuzzTest& other) : func(other.func), seeds(other.seeds) {}

    template <typename... T>
    using FuzzTestWithDomainType =
        FuzzTestWithDomain<TargetFunction, FuncType,
                           AggregateOf<std::tuple<typename T::ValueType...>, T...>>;

    template <typename... T>
    FuzzTestWithDomainType<T...> WithDomains(
        AggregateOf<std::tuple<typename T::ValueType...>, T...> domain) {
        return FuzzTestWithDomainType<T...>(*this, domain);
    }

    template <typename... T>
    FuzzTestWithDomainType<T...> WithDomains(T&&... domains) {
        return WithDomains(TupleOf(std::forward<T>(domains)...));
    }

    FuzzTest& WithSeeds(std::vector<ValueType> seeds) {
        this->seeds.insert(this->seeds.end(), seeds.begin(), seeds.end());
        return *this;
    }

    // This should create default domains
    virtual void        Run(int count = 1000, int seed = 0) {}
    virtual void        RunOneTime(const uint8_t* data, size_t size) {}
    virtual std::string MutateData(const uint8_t* data, size_t size, size_t max_size,
                                   unsigned int seed) {
        return "";
    }

    TargetFunction         func;
    std::vector<ValueType> seeds;
};

extern void RunFuzzTest(IFuzzTest& fuzz_test, int seed = 0, int runs = 1000, int max_len = 0,
                        int timeout = 1200, int len_control = 100);

template <typename TargetFunction, typename FuncType, typename Domain, typename Base>
struct FuzzTestWithDomain : public Base {
    FuzzTestWithDomain(const Base& ft, const Domain& domain) : Base(ft), domain(domain) {}

    virtual void Run(int count = 1000, int seed = 0) override {
        rng = new Rng(seed);
        RunFuzzTest(*this, seed, count);
        delete rng;
        rng = nullptr;
    }

    virtual void RunOneTime(const uint8_t* data, size_t size) override {
        std::string                 input  = std::string((const char*)data);
        IRObject                    obj    = IRObject::FromString(input);
        typename Domain::CorpusType corpus = domain.ParseCorpus(obj);
        typename Domain::ValueType  value  = domain.GetValue(corpus);
        std::apply(this->func, value);
    }

    virtual std::string MutateData(const uint8_t* data, size_t size, size_t max_size,
                           unsigned int seed) override {
        std::string                 input  = std::string((const char*)data);
        IRObject                    obj    = IRObject::FromString(input);
        typename Domain::CorpusType corpus = domain.ParseCorpus(obj);
        domain.Mutate(*rng, corpus, false);
        IRObject mutated_obj = domain.SerializeCorpus(corpus);
        return IRObject::ToString(mutated_obj);
    }

    Domain domain;
    Rng*   rng;
};


template <typename T>
FuzzTest<T> FuzzFunction(T func, TestContext* context) {
    return FuzzTest<T>(func);
}

template <typename T>
std::vector<T> ReadCorpusFromDir(std::string dir);


}  // namespace zeroerr


ZEROERR_SUPPRESS_COMMON_WARNINGS_POP