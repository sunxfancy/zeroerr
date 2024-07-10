#pragma once

#include "zeroerr/internal/config.h"
#include "zeroerr/unittest.h"

#include "zeroerr/domains/aggregate_of.h"
#include "zeroerr/domains/arbitrary.h"
#include "zeroerr/domains/container_of.h"
#include "zeroerr/domains/element_of.h"
#include "zeroerr/domains/in_range.h"
#include "zeroerr/internal/rng.h"


#include <exception>
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

/**
 * @brief FunctionDecomposition is a meta function to decompose the function type.
 *        The ValueType and numOfArgs will be the result of the decomposition.
 */
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


/**
 * @brief FuzzTest is a template class to create a fuzz test for the target function.
 * @tparam TargetFunction The target function to fuzz.
 * @tparam FuncType is the decomposition result of the target function.
 */
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

    int  count = 0, max_count = 0;
    bool should_stop() { return count == max_count; }
};

class FuzzFinishedException : public std::exception {
public:
    virtual const char* what() const throw() { return "Fuzzing finished"; }
};

// The details are described in the declaration of the class.
template <typename TargetFunction, typename FuncType>
struct FuzzTest : IFuzzTest {
    using ValueType = typename FuncType::ValueType;
    FuzzTest(TargetFunction func, TestContext* context) : func(func), context(context) {}
    FuzzTest(const FuzzTest& other)
        : func(other.func), context(other.context), seeds(other.seeds) {}

    template <typename... T>
    using FuzzTestWithDomainType =
        FuzzTestWithDomain<TargetFunction, FuncType,
                           AggregateOf<std::tuple<typename T::ValueType...>, T...>>;

    /**
     * @tparam T The domain type to use.
     */
    template <typename... T>
    FuzzTestWithDomainType<T...> WithDomains(
        AggregateOf<std::tuple<typename T::ValueType...>, T...> domain) {
        return FuzzTestWithDomainType<T...>(*this, domain);
    }

    /**
     * @tparam T The domain type to use.
     * This function will decompose the domain type and use tuple to represent the list of domains.
     * The previous function is matched in this call.
     */
    template <typename... T>
    FuzzTestWithDomainType<T...> WithDomains(T&&... domains) {
        return WithDomains(TupleOf(std::forward<T>(domains)...));
    }

    FuzzTest& WithSeeds(std::vector<ValueType> _seeds) {
        this->seeds.insert(this->seeds.end(), _seeds.begin(), _seeds.end());
        return *this;
    }

    // This should create default domains
    virtual void        Run(int = 1000, int = 0) {}
    virtual void        RunOneTime(const uint8_t*, size_t) {}
    virtual std::string MutateData(const uint8_t*, size_t, size_t, unsigned int) { return ""; }

    TargetFunction         func;
    TestContext*           context;
    std::vector<ValueType> seeds;
};

extern void RunFuzzTest(IFuzzTest& fuzz_test, int seed = 0, int runs = 1000, int max_len = 0,
                        int timeout = 1200, int len_control = 100);


/**
 * @brief FuzzTestWithDomain implements the Base class with the domain passed into.
 * @tparam TargetFunction The target function to fuzz.
 * @tparam FuncType is the decomposition result of the target function.
 * @tparam Domain The domain to use.
 * @tparam Base The base class to inherit from.
 */
template <typename TargetFunction, typename FuncType, typename Domain, typename Base>
struct FuzzTestWithDomain : public Base {
    FuzzTestWithDomain(const Base& ft, const Domain& domain) : Base(ft), m_domain(domain) {}

    virtual void Run(int _count = 1000, int _seed = 0) override {
        Base::count     = 1;
        Base::max_count = _count;
        m_rng           = new Rng(_seed);
        RunFuzzTest(*this, _seed, _count, 500, 1200, 1);
        delete m_rng;
        m_rng = nullptr;
    }

    typename Domain::CorpusType GetRandomCorpus() {
        Rng& rng = *this->m_rng;
        if (Base::seeds.size() > 0 && rng.bounded(2) == 0) {
            return m_domain.FromValue(Base::seeds[rng() % Base::seeds.size()]);
        }
        return m_domain.GetRandomCorpus(rng);
    }

    typename Domain::CorpusType TryParse(const std::string& input) {
        try {
            IRObject obj = IRObject::FromString(input);
            if (obj.type == IRObject::Type::Undefined) return GetRandomCorpus();
            return m_domain.ParseCorpus(obj);
        } catch (...) {
            return GetRandomCorpus();
        }
    }

    template <typename T, unsigned... I>
    void apply(T args, detail::seq<I...>) {
        this->func(std::get<I>(args)...);
    }

    virtual void RunOneTime(const uint8_t* data, size_t size) override {
        Base::count++;
        std::string                 input  = std::string((const char*)data, size);
        typename Domain::CorpusType corpus = TryParse(input);
        typename Domain::ValueType  value  = m_domain.GetValue(corpus);
        apply(value, detail::gen_seq<std::tuple_size<typename Domain::ValueType>::value>{});
    }

    virtual std::string MutateData(const uint8_t* data, size_t size, size_t max_size,
                                   unsigned int seed) override {
        Rng                         rng(seed);
        std::string                 input  = std::string((const char*)data, size);
        typename Domain::CorpusType corpus = TryParse(input);
        m_domain.Mutate(rng, corpus, false);
        IRObject mutated_obj = m_domain.SerializeCorpus(corpus);
        return IRObject::ToString(mutated_obj);
    }

    Domain m_domain;
    Rng*   m_rng;
};


template <typename T>
FuzzTest<T> FuzzFunction(T func, TestContext* context) {
    return FuzzTest<T>(func, context);
}

template <typename T>
std::vector<T> ReadCorpusFromDir(std::string dir);


}  // namespace zeroerr


ZEROERR_SUPPRESS_COMMON_WARNINGS_POP