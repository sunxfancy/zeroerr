#pragma once
#include <tuple>
#include "zeroerr/internal/config.h"

#include "zeroerr/internal/typetraits.h"

#include <cstdint>
#include <stdexcept>
#include <cstring>
#include "zeroerr/string.h"
#include <type_traits>
#include <vector>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

/**
 * @brief Interface for serializable objects
 * 
 * IRObject (Intermediate Representation Object) provides a low-level interface for serializing 
 * data types into a common format. It uses a union to store different data types and provides
 * type-safe access through templated getter methods.
 *
 * The object can store:
 * - Integers (int64_t)
 * - Floating point numbers (double) 
 * - Strings (char* for long strings, char[8] for short strings)
 * - Nested objects (IRObject*)
 *
 * Memory management:
 * - The object takes ownership of allocated strings and nested objects
 * - Copy/move operations perform deep copies/moves
 * - The destructor frees any owned memory
 *
 * Usage example:
 * @code
 * IRObject obj;
 * obj.type = IRObject::Int;
 * obj.i = 42;
 * int value = obj.GetScalar<int>(); // value = 42
 * @endcode
 */

struct IRObject {
    IRObject() noexcept : i(0), others{}, type(Undefined) {}
    ~IRObject();
    IRObject(const IRObject& other);
    IRObject(IRObject&& other) noexcept;
    IRObject& operator=(const IRObject& other);
    IRObject& operator=(IRObject&& other) noexcept;
    void Reset() noexcept;

    enum Type { Undefined = 0, Int, Float, String, ShortString, Object };

    union {
        int64_t   i;
        double    f;
        char*     s;
        char      ss[8];
        IRObject* o;  // first must be the number of elements
    };
    char     others[7];
    unsigned char type;

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, T>::type GetScalar() const {
        return static_cast<T>(i);
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, T>::type GetScalar() const {
        return static_cast<T>(f);
    }

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, T>::type GetScalar() const {
        return GetScalar<typename std::underlying_type<T>::type>();
    }

    template <typename T>
    typename std::enable_if<detail::StringLike<T>::owning, T>::type GetScalar() const {
        if (type == Type::String)
            return T(s);
        else if (type == Type::ShortString)
            return T(ss);
        return "";
    }

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type SetScalar(T val) {
        Reset();
        i    = static_cast<int64_t>(val);
        type = Type::Int;
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, void>::type SetScalar(T val) {
        Reset();
        f    = static_cast<double>(val);
        type = Type::Float;
    }

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type SetScalar(T val) {
        SetScalar<typename std::underlying_type<T>::type>(val);
    }

    template <typename T>
    typename std::enable_if<detail::StringLike<T>::owning, void>::type SetScalar(T val) {
        IRObject next;
        size_t size = val.size();
        // Keep the terminator inside ss; adjacent padding is not part of the array.
        if (size >= sizeof(next.ss)) {
            next.s = alloc_str(size);
            std::memcpy(next.s, val.data(), size);
            next.type = Type::String;
        } else {
            std::memcpy(next.ss, val.data(), size);
            next.ss[size] = 0;
            next.type = Type::ShortString;
        }
        *this = std::move(next);
    }

    void SetScalar(const IRObject& obj) { *this = obj; }

    struct Childrens {
        int64_t   size;
        IRObject* children;
    };
    Childrens GetChildren() const {
        if (type != Type::Object) return {0, nullptr};
        return {o->i, o + 1};
    }

    void SetChildren(IRObject* children) {
        if (!children) throw std::invalid_argument("IRObject children cannot be null");
        if (type == Type::Object && o == children - 1) return;
        Reset();
        o    = children - 1;
        type = Type::Object;
    }

    // ================================================================

    template <typename T>
    static
        typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value ||
                                    detail::StringLike<T>::owning || std::is_enum<T>::value,
                                IRObject>::type
        FromCorpus(T val) {
        IRObject obj;
        obj.SetScalar(val);
        return obj;
    }

    template <typename T>
    static typename std::enable_if<
        detail::is_container<T>::value && !detail::StringLike<T>::owning, IRObject>::type
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
    ToCorpus(const IRObject& obj) {
        if (obj.type != IRObject::Int) {
            throw std::runtime_error("Invalid type conversion.");
        }
        return obj.GetScalar<T>();
    }

    template <typename T>
    static typename std::enable_if<std::is_floating_point<T>::value, T>::type ToCorpus(
        const IRObject& obj) {
        if (obj.type != IRObject::Float) {
            throw std::runtime_error("Invalid type conversion.");
        }
        return obj.GetScalar<T>();
    }

    template <typename T>
    static typename std::enable_if<detail::StringLike<T>::owning, T>::type ToCorpus(
        const IRObject& obj) {
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
        detail::is_container<T>::value && !detail::StringLike<T>::owning, T>::type
    ToCorpus(const IRObject& obj) {
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
    ToCorpus(const IRObject& obj) {
        if (obj.type != IRObject::Object || obj.GetChildren().size != std::tuple_size<T>::value) {
            throw std::runtime_error("Invalid type conversion.");
        }
        return parse_tuple<T>(obj.o + 1, detail::gen_seq<std::tuple_size<T>::value>{});
    }

    template <typename T>
    static typename std::enable_if<detail::is_specialization<T, std::pair>::value, T>::type
    ToCorpus(const IRObject& obj) {
        if (obj.type != IRObject::Object || obj.GetChildren().size != 2) {
            throw std::runtime_error("Invalid type conversion.");
        }
        return std::make_pair(ToCorpus<typename T::first_type>(obj.GetChildren().children[0]),
                              ToCorpus<typename T::second_type>(obj.GetChildren().children[1]));
    }


    static char*     alloc_str(size_t size);
    static IRObject* alloc(size_t size);

    // Serialize the object as a string.
    static zeroerr::String ToString(const IRObject& obj);
    static IRObject    FromString(zeroerr::String str);

    static std::vector<uint8_t> ToBinary(IRObject obj);
    static IRObject             FromBinary(std::vector<uint8_t> bin);

private:
    void moveFrom(IRObject& other) noexcept;
};


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP