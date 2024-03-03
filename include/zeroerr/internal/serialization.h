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
    std::enable_if<std::is_integral<T>::value, T> GetScalar() {
        return static_cast<T>(i);
    }

    template <typename T>
    std::enable_if<std::is_floating_point<T>::value, T> GetScalar() {
        return static_cast<T>(f);
    }

    template <typename T>
    std::enable_if<std::is_enum<T>::value, T> GetScalar() {
        return GetScalar<typename std::underlying_type<T>::type>();
    }

    template <typename T>
    std::enable_if<std::is_same<T, std::string>::value, T> GetScalar() {
        if (s & 1)
            return std::string((char*)(s & ~1));
        else
            return std::string(ss);
    }

    template <typename T>
    std::enable_if<std::is_integral<T>::value, void> SetScalar(T val) {
        i = static_cast<int64_t>(val);
    }

    template <typename T>
    std::enable_if<std::is_floating_point<T>::value, void> SetScalar(T val) {
        f = static_cast<double>(val);
    }

    template <typename T>
    std::enable_if<std::is_enum<T>::value, void> SetScalar(T val) {
        SetScalar<typename std::underlying_type<T>::type>(val);
    }

    template <typename T>
    std::enable_if<std::is_same<T, std::string>::value, void> SetScalar(T val) {
        unsigned size = val.size();
        if (size > 7) {
            s = (char*)(alloc_str(size) | 1);
            strcpy(s, val.c_str());
        } else {
            strcpy(ss, val.c_str());
            ss[size] = 0;
        }
    }

    void SetScalar(const IRObject& obj) {
        i = obj.i;
    }

    struct Childrens {
        int64_t size;
        IRObject* children;
    };
    Childrens GetChildren() {
        return {o->i, o+1};
    }

    void SetChildren(IRObject* children) {
        o = children-1;
    }

    // ================================================================

    template <typename T> static 
    typename std::enable_if<
        std::is_integral<T>::value || 
        std::is_floating_point<T>::value ||
        std::is_same<T, std::string>::value ||
        std::is_enum<T>::value, IRObject>::type
    FromCorpus(T val) {
        IRObject obj;
        obj.SetScalar(val);
        return obj;
    }

    template <typename T> static 
    typename std::enable_if<
        detail::is_container<T>::value, IRObject>::type
    FromCorpus(const T& val) {
        unsigned size = val.size();

        IRObject* children = alloc(size);
        SetChildren(children);
        
        for (const auto& elem : value) {
            *children++ = IRObject::FromCorpus(elem);
        }
    }


    template <class TupType, unsigned... I>
    inline void handle_tuple(const TupType& _tup, IRObject* children, detail::seq<I...>) {
        int _[] = {((children+I)->SetScala(FromCorpus(std::get<I>(_tup))), 0)...};
        (void)_;
    }

    template <typename... Args> static
    IRObject FromCorpus(const std::tuple<Args...>& val) {
        unsigned size = sizeof...(Args);

        IRObject* children = alloc(size);
        SetChildren(children);
        
        handle_tuple(val, children, detail::gen_seq<sizeof...(Args)>{});
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