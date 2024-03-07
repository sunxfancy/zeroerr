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
    IRObject() { std::memset(this, 0, sizeof(IRObject)); }
    ~IRObject() {}
    IRObject(const IRObject& other) { *this = other; }
    IRObject(IRObject&& other) { *this = std::move(other); }
    IRObject& operator=(const IRObject& other) {
        std::memcpy(this, &other, sizeof(IRObject));
        return *this;
    }
    IRObject& operator=(IRObject&& other) {
        std::memcpy(this, &other, sizeof(IRObject));
        std::memset(&other, 0, sizeof(IRObject));
        return *this;
    }

    enum Type { Undefined, Int, Float, String, ShortString, Object };

    union {
        int64_t   i;
        double    f;
        char*     s;
        char      ss[8];
        IRObject* o;  // first must be the number of elements
    };
    char     others[7];
    unsigned type;

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
        if (type == Type::String)
            return std::string(s);
        else if (type == Type::ShortString)
            return std::string(ss);
        return "";
    }

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type SetScalar(T val) {
        i    = static_cast<int64_t>(val);
        type = Type::Int;
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type SetScalar(T val) {
        f    = static_cast<double>(val);
        type = Type::Float;
    }

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type SetScalar(T val) {
        SetScalar<typename std::underlying_type<T>::type>(val);
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value, void>::type SetScalar(T val) {
        size_t size = val.size();
        if (size > 14) {
            s = alloc_str(size);
            strcpy(s, val.c_str());
            type = Type::String;
        } else {
            strcpy(ss, val.c_str());
            ss[size] = 0;
            type     = Type::ShortString;
        }
    }

    void SetScalar(const IRObject& obj) { *this = obj; }

    struct Childrens {
        int64_t   size;
        IRObject* children;
    };
    Childrens GetChildren() { return {o->i, o + 1}; }

    void SetChildren(IRObject* children) {
        o    = children - 1;
        type = Type::Object;
    }

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
        IRObject* children = alloc(val.size());
        IRObject  obj;
        obj.SetChildren(children);

        for (const auto& elem : val) {
            *children++ = IRObject::FromCorpus(elem);
        }
        return obj;
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

    template <typename T1, typename T2>
    static IRObject FromCorpus(const std::pair<T1, T2>& val) {
        IRObject  obj;
        IRObject* children = alloc(2);
        obj.SetChildren(children);
        children[0] = IRObject::FromCorpus(val.first);
        children[1] = IRObject::FromCorpus(val.second);
        return obj;
    }

    template <typename T>
    static typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value, T>::type
    ToCorpus(IRObject obj) {
        if (obj.type != IRObject::Int) {
            throw std::runtime_error("Invalid type conversion.");
        }
        return obj.GetScalar<T>();
    }

    template <typename T>
    static typename std::enable_if<std::is_floating_point<T>::value, T>::type ToCorpus(
        IRObject obj) {
        if (obj.type != IRObject::Float) {
            throw std::runtime_error("Invalid type conversion.");
        }
        return obj.GetScalar<T>();
    }

    template <typename T>
    static typename std::enable_if<std::is_same<T, std::string>::value, T>::type ToCorpus(
        IRObject obj) {
        if (obj.type != IRObject::String && obj.type != IRObject::ShortString) {
            throw std::runtime_error("Invalid type conversion.");
        }
        return obj.GetScalar<T>();
    }


    template <typename TupType, unsigned... I>
    static TupType parse_tuple(IRObject* children, detail::seq<I...>) {
        return std::make_tuple(
            ToCorpus<typename std::tuple_element<I, TupType>::type>(*(children + I))...);
    }

    template <typename T>
    static typename std::enable_if<
        detail::is_container<T>::value && !std::is_same<T, std::string>::value, T>::type
    ToCorpus(IRObject obj) {
        if (obj.type != IRObject::Object) {
            throw std::runtime_error("Invalid type conversion.");
        }

        auto c = obj.GetChildren();

        T val;
        for (int i = 0; i < c.size; i++) {
            val.insert(val.end(), ToCorpus<typename T::value_type>(c.children[i]));
        }
        return val;
    }


    template <typename T>
    static typename std::enable_if<detail::is_specialization<T, std::tuple>::value, T>::type
    ToCorpus(IRObject obj) {
        if (obj.type != IRObject::Object || obj.GetChildren().size != std::tuple_size<T>::value) {
            throw std::runtime_error("Invalid type conversion.");
        }
        return parse_tuple<T>(obj.o + 1, detail::gen_seq<std::tuple_size<T>::value>{});
    }

    template <typename T>
    static typename std::enable_if<detail::is_specialization<T, std::pair>::value, T>::type
    ToCorpus(IRObject obj) {
        if (obj.type != IRObject::Object || obj.GetChildren().size != 2) {
            throw std::runtime_error("Invalid type conversion.");
        }
        return std::make_pair(ToCorpus<typename T::first_type>(obj.GetChildren().children[0]),
                              ToCorpus<typename T::second_type>(obj.GetChildren().children[1]));
    }


    static char*     alloc_str(size_t size);
    static IRObject* alloc(size_t size);

    // Serialize the object as a string.
    static std::string ToString(IRObject obj);
    static IRObject    FromString(std::string str);

    static std::vector<uint8_t> ToBinary(IRObject obj);
    static IRObject             FromBinary(std::vector<uint8_t> bin);
};


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP