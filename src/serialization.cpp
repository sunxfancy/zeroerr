#include "zeroerr/internal/serialization.h"
#include "zeroerr/assert.h"

#include <sstream>
namespace zeroerr {

IRObject* IRObject::alloc(unsigned size) {
    IRObject* list = new IRObject[size + 1];
    list[0].i      = size;
    return list + 1;
}

char* IRObject::alloc_str(unsigned size) {
    char* s = new char[size + 1];
    s[size] = 0;
    return s;
}

static std::string escape(std::string str) {
    std::string result;
    for (char c : str) {
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

static std::string unescape(std::string str) {
    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\\') {
            switch (str[++i]) {
                case 's': result += ' '; break;
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case 'f': result += '\f'; break;
                case 'v': result += '\v'; break;
                case '\\': result += '\\'; break;
                case '"': result += '"'; break;
                case 'x': {
                    char c = 0;
                    for (int j = 0; j < 2; ++j) {
                        c *= 16;
                        if (str[i + 1] >= '0' && str[i + 1] <= '9') {
                            c += str[i + 1] - '0';
                        } else if (str[i + 1] >= 'a' && str[i + 1] <= 'f') {
                            c += str[i + 1] - 'a' + 10;
                        } else if (str[i + 1] >= 'A' && str[i + 1] <= 'F') {
                            c += str[i + 1] - 'A' + 10;
                        }
                    }
                    result += c;
                    i += 2;
                }
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

static void to_string(IRObject obj, std::stringstream& ss) {
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

static IRObject from_string(std::stringstream& ss, std::string& token) {
    IRObject obj;
    if (token == "{") {
        std::vector<IRObject> children;
        while (ss >> token) {
            if (token == "}") break;
            children.push_back(from_string(ss, token));
        }
        IRObject* child = IRObject::alloc(children.size());
        for (unsigned i = 0; i < children.size(); ++i) {
            child[i] = children[i];
        }
        obj.SetChildren(child);
        return obj;
    }
    if (token[0] == '"') {
        CHECK(token.size() > 1 AND token.back() == '"');
        obj.SetScalar(unescape(token.substr(1, token.size() - 2)));
        return obj;
    }
    if (token.back() == 'f') {
        obj.SetScalar(std::stod(token.substr(0, token.size() - 1)));
        return obj;
    }
    obj.SetScalar(std::stoll(token));
    return obj;
}

std::string IRObject::ToString(IRObject obj) {
    std::stringstream ss;
    to_string(obj, ss);
    return ss.str();
}

IRObject IRObject::FromString(std::string str) {
    std::stringstream ss(str);
    std::string       token;
    ss >> token;
    return from_string(ss, token);
}

std::vector<uint8_t> IRObject::ToBinary(IRObject obj) {
    std::vector<uint8_t> bin;
    return bin;
}

IRObject IRObject::FromBinary(std::vector<uint8_t> bin) {
    IRObject obj;
    return obj;
}


}  // namespace zeroerr
