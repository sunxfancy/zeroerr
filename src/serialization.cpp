#include "zeroerr/internal/serialization.h"

#include <sstream>
namespace zeroerr {

void IRObject::Reset() noexcept {
    if (type == Type::String) delete[] s;
    if (type == Type::Object) delete[] o;
    i = 0;
    type = Type::Undefined;
}

IRObject::~IRObject() { Reset(); }

IRObject::IRObject(const IRObject& other) : IRObject() {
    if (other.type == Type::String) {
        const size_t size = std::strlen(other.s);
        s = alloc_str(size);
        std::memcpy(s, other.s, size + 1);
        type = Type::String;
    } else if (other.type == Type::Object) {
        IRObject copy;
        auto children = other.GetChildren();
        copy.SetChildren(alloc(static_cast<size_t>(children.size)));
        for (int64_t j = 0; j < children.size; ++j)
            copy.o[j + 1] = children.children[j];
        *this = std::move(copy);
    } else if (other.type == Type::ShortString) {
        std::memcpy(ss, other.ss, sizeof(ss));
        type = other.type;
    } else {
        if (other.type == Type::Float) f = other.f;
        else i = other.i;
        type = other.type;
    }
}

IRObject::IRObject(IRObject&& other) noexcept : IRObject() { moveFrom(other); }

IRObject& IRObject::operator=(const IRObject& other) {
    if (this != &other) {
        IRObject copy(other);
        *this = std::move(copy);
    }
    return *this;
}

IRObject& IRObject::operator=(IRObject&& other) noexcept {
    if (this == &other) return *this;
    // Detach first: other may be a child owned by this object.
    IRObject incoming(std::move(other));
    Reset();
    moveFrom(incoming);
    return *this;
}

void IRObject::moveFrom(IRObject& other) noexcept {
    switch (other.type) {
        case Type::String: s = other.s; break;
        case Type::Object: o = other.o; break;
        case Type::ShortString: std::memcpy(ss, other.ss, sizeof(ss)); break;
        case Type::Float: f = other.f; break;
        default: i = other.i; break;
    }
    type = other.type;
    other.type = Type::Undefined;
    other.i = 0;
}

IRObject* IRObject::alloc(size_t size) {
    IRObject* list = new IRObject[size + 1];
    list[0].i      = size;
    return list + 1;
}

char* IRObject::alloc_str(size_t size) {
    char* s = new char[size + 1];
    s[size] = 0;
    return s;
}

static zeroerr::String escape(const zeroerr::String& str) {
    zeroerr::String result;
    for (unsigned char c : str) {
        switch (c) {
            case ' ': result += "\\s"; break;
            case '\n': result += "\\n"; break;
            case '\t': result += "\\t"; break;
            case '\r': result += "\\r"; break;
            case '\f': result += "\\f"; break;
            case '\v': result += "\\v"; break;
            case '\\': result += "\\\\"; break;
            case '"': result += "\\\""; break;
            default: {
                if (c < 32 || c > 126) {
                    result += "\\x";
                    result += "0123456789abcdef"[c >> 4];
                    result += "0123456789abcdef"[c & 15];
                    continue;
                }
                result += c;
            }
        }
    }
    return result;
}

static unsigned hexDigit(char c) {
    if (c >= '0' && c <= '9') return static_cast<unsigned>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<unsigned>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<unsigned>(c - 'A' + 10);
    throw std::invalid_argument("Invalid hexadecimal escape");
}

static zeroerr::String unescape(const zeroerr::String& str) {
    zeroerr::String result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] != '\\') {
            result += str[i];
            continue;
        }
        if (++i == str.size()) throw std::invalid_argument("Incomplete escape");
        switch (str[i]) {
            case 's': result += ' '; break;
            case 'n': result += '\n'; break;
            case 't': result += '\t'; break;
            case 'r': result += '\r'; break;
            case 'f': result += '\f'; break;
            case 'v': result += '\v'; break;
            case '\\': result += '\\'; break;
            case '"': result += '"'; break;
            case 'x':
                if (str.size() - i <= 2) throw std::invalid_argument("Incomplete hexadecimal escape");
                result += static_cast<char>((hexDigit(str[i + 1]) << 4) | hexDigit(str[i + 2]));
                i += 2;
                break;
            default: throw std::invalid_argument("Unknown escape");
        }
    }
    return result;
}

static void to_string(const IRObject& obj, std::stringstream& ss) {
    switch (obj.type) {
        case IRObject::Type::Int: ss << obj.i; break;
        case IRObject::Type::Float: ss << obj.f << 'f'; break;
        case IRObject::Type::String: ss << '"' << escape(obj.s) << '"'; break;
        case IRObject::Type::ShortString: ss << '"' << escape(obj.ss) << '"'; break;
        case IRObject::Type::Object:
            ss << "{ ";
            auto c = obj.GetChildren();
            for (unsigned i = 0; i < c.size; ++i) {
                to_string(*(c.children + i), ss);
                ss << " ";
            }
            ss << "}";
            break;
    }
}

static IRObject from_string(std::stringstream& ss, zeroerr::String& token) {
    IRObject obj;
    if (token.empty()) return obj;
    if (token == "{") {
        std::vector<IRObject> children;
        while (ss >> token) {
            if (token.empty()) return obj;
            if (token == "}") break;
            IRObject child = from_string(ss, token);
            if (child.type == IRObject::Type::Undefined)
                return obj;
            children.push_back(std::move(child));
        }
        IRObject* child = IRObject::alloc(children.size());
        obj.SetChildren(child);
        for (unsigned i = 0; i < children.size(); ++i) {
            child[i] = std::move(children[i]);
        }
        return obj;
    }
    if (token[0] == '"') {
        if (token.size() < 2 || token.back() != '"')
            throw std::invalid_argument("Unterminated string");
        obj.SetScalar(unescape(token.substr(1, token.size() - 2)));
        return obj;
    }
    if (token.back() == 'f') {
        obj.SetScalar(std::stod(token.substr(0, token.size() - 1)));
        return obj;
    }
    if (token[0] == '-' || (token[0] >= '0' && token[0] <= '9')) {
        obj.SetScalar(std::stoll(token));
        return obj;
    }
    return obj;
}

zeroerr::String IRObject::ToString(const IRObject& obj) {
    std::stringstream ss;
    to_string(obj, ss);
    return ss.str();
}

IRObject IRObject::FromString(zeroerr::String str) {
    std::stringstream ss(str);
    zeroerr::String       token;
    ss >> token;
    return from_string(ss, token);
}

std::vector<uint8_t> IRObject::ToBinary(IRObject /*obj*/) {
    std::vector<uint8_t> bin;
    return bin;
}

IRObject IRObject::FromBinary(std::vector<uint8_t> /*bin*/) {
    IRObject obj;
    return obj;
}


}  // namespace zeroerr
