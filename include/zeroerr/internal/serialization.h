#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/internal/typetraits.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {


struct IRObject {
    union {
        int64_t   i;
        double    f;
        char*     s;
        char      ss[8];
        IRObject* o;  // first must be the number of elements
    };

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, T>::type GetScalar() {
        return static_cast<T>(i);
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, T>::type GetScalar() {
        return static_cast<T>(f);
    }

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, T>::type GetScalar() {
        return GetScalar<typename std::underlying_type<T>::type>();
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value, T>::type GetScalar() {
        if ((uint64_t)s & 1)
            return std::string((char*)((uint64_t)s & ~1));
        else
            return std::string(ss);
    }

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type SetScalar(T val) {
        i = static_cast<int64_t>(val);
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type SetScalar(T val) {
        f = static_cast<double>(val);
    }

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type SetScalar(T val) {
        SetScalar<typename std::underlying_type<T>::type>(val);
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value, void>::type SetScalar(T val) {
        unsigned size = val.size();
        if (size > 7) {
            s = (char*)((uint64_t)alloc_str(size) | 1);
            strcpy(s, val.c_str());
        } else {
            strcpy(ss, val.c_str());
            ss[size] = 0;
        }
    }

    void SetScalar(const IRObject& obj) { i = obj.i; }

    struct Childrens {
        int64_t   size;
        IRObject* children;
    };
    Childrens GetChildren() { return {o->i, o + 1}; }

    void SetChildren(IRObject* children) { o = children - 1; }

    // ================================================================

    template <typename T>
    static
        typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value ||
                                    std::is_same<T, std::string>::value || std::is_enum<T>::value,
                                IRObject>::type
        FromCorpus(T val) {
        IRObject obj;
        obj.SetScalar(val);
        return obj;
    }

    template <typename T>
    static typename std::enable_if<
        detail::is_container<T>::value && !std::is_same<T, std::string>::value, IRObject>::type
    FromCorpus(const T& val) {
        unsigned size = val.size();

        IRObject* children = alloc(size);
        val.SetChildren(children);

        for (const auto& elem : val) {
            *children++ = IRObject::FromCorpus(elem);
        }
    }


    template <class TupType, unsigned... I>
    inline static void handle_tuple(const TupType& _tup, IRObject* children, detail::seq<I...>) {
        int _[] = {((children + I)->SetScalar(FromCorpus(std::get<I>(_tup))), 0)...};
        (void)_;
    }

    template <typename... Args>
    static IRObject FromCorpus(const std::tuple<Args...>& val) {
        unsigned  size = sizeof...(Args);
        IRObject  obj;
        IRObject* children = alloc(size);
        obj.SetChildren(children);

        handle_tuple(val, children, detail::gen_seq<sizeof...(Args)>{});
        return obj;
    }

    template <typename T>
    static
        typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value ||
                                    std::is_same<T, std::string>::value || std::is_enum<T>::value,
                                T>::type
        ToCorpus(IRObject obj) {
        return obj.GetScalar<T>();
    }

    template <typename TupType, unsigned... I>  
    static void parse_tuple(TupType& _tup, IRObject* children, detail::seq<I...>) {
        int _[] = {((std::get<I>(_tup) = ToCorpus<typename std::tuple_element<I, TupType>::type>(
                       *(children + I))),
                    0)...};
        (void)_;
    }


    template <typename T> static
    typename std::enable_if<detail::is_specialization<T, std::tuple>::value, T>::type
    ToCorpus(IRObject obj) {
        unsigned size = obj.o->i;
        T tup;
        parse_tuple(tup, obj.o + 1, detail::gen_seq<std::tuple_size<T>::value>{});
        return tup;
    }


    static char*     alloc_str(unsigned size);
    static IRObject* alloc(unsigned size);

    // Serialize the object as a string.
    static std::string ToString(IRObject obj);
    static IRObject    FromString(std::string str);

    static std::vector<uint8_t> ToBinary(IRObject obj);
    static IRObject             FromBinary(std::vector<uint8_t> bin);
};


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP