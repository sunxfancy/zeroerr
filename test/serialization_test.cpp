#include "zeroerr/assert.h"
#include "zeroerr/internal/serialization.h"
#include "zeroerr/unittest.h"

using namespace zeroerr;

TEST_CASE("IRObject static assert") {
    IRObject obj;
    CHECK(obj.type == IRObject::Type::Undefined);
    CHECK(obj.GetChildren().size == 0);
    CHECK(obj.GetChildren().children == nullptr);
    CHECK(sizeof(IRObject) == 16);
}

TEST_CASE("IRObject basic integer serialization") {
    IRObject obj;
    obj.SetScalar(42);
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "42");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.type == IRObject::Type::Int);
    CHECK(parsed.GetScalar<int>() == 42);
}

TEST_CASE("IRObject negative integer serialization") {
    IRObject obj;
    obj.SetScalar(-123);
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "-123");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.type == IRObject::Type::Int);
    CHECK(parsed.GetScalar<int>() == -123);
}

TEST_CASE("IRObject float serialization") {
    IRObject obj;
    obj.SetScalar(3.14);
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "3.14f");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.type == IRObject::Type::Float);
    CHECK(parsed.GetScalar<double>() == 3.14);
}

TEST_CASE("IRObject string serialization") {
    IRObject obj;
    obj.SetScalar(std::string("hello world"));
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "\"hello\\sworld\"");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.GetScalar<std::string>() == "hello world");
}

TEST_CASE("IRObject short string serialization") {
    IRObject obj;
    obj.SetScalar(std::string("short"));  // <= 14 characters
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "\"short\"");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.type == IRObject::Type::ShortString);
    CHECK(parsed.GetScalar<std::string>() == "short");
}

TEST_CASE("IRObject string with newline") {
    IRObject obj;
    obj.SetScalar(std::string("line1\nline2"));
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "\"line1\\nline2\"");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.GetScalar<std::string>() == "line1\nline2");
}

TEST_CASE("IRObject string with tab") {
    IRObject obj;
    obj.SetScalar(std::string("col1\tcol2"));
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "\"col1\\tcol2\"");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.GetScalar<std::string>() == "col1\tcol2");
}

TEST_CASE("IRObject string with special characters") {
    IRObject obj;
    obj.SetScalar(std::string("test\r\n\t\f\v"));
    
    std::string str = IRObject::ToString(obj);
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.GetScalar<std::string>() == "test\r\n\t\f\v");
}

TEST_CASE("IRObject string with backslash") {
    IRObject obj;
    obj.SetScalar(std::string("path\\to\\file"));
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "\"path\\\\to\\\\file\"");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.GetScalar<std::string>() == "path\\to\\file");
}

TEST_CASE("IRObject string with quotes") {
    IRObject obj;
    obj.SetScalar(std::string("say \"hello\""));
    
    std::string str = IRObject::ToString(obj);
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.GetScalar<std::string>() == "say \"hello\"");
}

TEST_CASE("IRObject empty object") {
    IRObject* children = IRObject::alloc(0);
    IRObject obj;
    obj.SetChildren(children);
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "{ }");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.type == IRObject::Type::Object);
    CHECK(parsed.GetChildren().size == 0);
}

TEST_CASE("IRObject with single child") {
    IRObject* children = IRObject::alloc(1);
    children[0].SetScalar(100);
    
    IRObject obj;
    obj.SetChildren(children);
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "{ 100 }");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.type == IRObject::Type::Object);
    auto c = parsed.GetChildren();
    CHECK(c.size == 1);
    CHECK(c.children[0].GetScalar<int>() == 100);
}

TEST_CASE("IRObject with multiple children") {
    IRObject* children = IRObject::alloc(3);
    children[0].SetScalar(1);
    children[1].SetScalar(2);
    children[2].SetScalar(3);
    
    IRObject obj;
    obj.SetChildren(children);
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "{ 1 2 3 }");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.type == IRObject::Type::Object);
    auto c = parsed.GetChildren();
    CHECK(c.size == 3);
    CHECK(c.children[0].GetScalar<int>() == 1);
    CHECK(c.children[1].GetScalar<int>() == 2);
    CHECK(c.children[2].GetScalar<int>() == 3);
}

TEST_CASE("IRObject nested objects") {
    // Create inner object { 10 20 }
    IRObject* inner_children = IRObject::alloc(2);
    inner_children[0].SetScalar(10);
    inner_children[1].SetScalar(20);
    
    // Create outer object { 1 { 10 20 } 2 }
    IRObject* outer_children = IRObject::alloc(3);
    outer_children[0].SetScalar(1);
    outer_children[1].SetChildren(inner_children);
    outer_children[2].SetScalar(2);
    
    IRObject obj;
    obj.SetChildren(outer_children);
    
    std::string str = IRObject::ToString(obj);
    CHECK(str == "{ 1 { 10 20 } 2 }");
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.type == IRObject::Type::Object);
    auto outer = parsed.GetChildren();
    CHECK(outer.size == 3);
    CHECK(outer.children[0].GetScalar<int>() == 1);
    CHECK(outer.children[1].type == IRObject::Type::Object);
    CHECK(outer.children[2].GetScalar<int>() == 2);
    
    auto inner = outer.children[1].GetChildren();
    CHECK(inner.size == 2);
    CHECK(inner.children[0].GetScalar<int>() == 10);
    CHECK(inner.children[1].GetScalar<int>() == 20);
}

TEST_CASE("IRObject with mixed types") {
    IRObject* children = IRObject::alloc(3);
    children[0].SetScalar(42);
    children[1].SetScalar(3.14);
    children[2].SetScalar(std::string("test"));
    
    IRObject obj;
    obj.SetChildren(children);
    
    std::string str = IRObject::ToString(obj);
    
    IRObject parsed = IRObject::FromString(str);
    CHECK(parsed.type == IRObject::Type::Object);
    auto c = parsed.GetChildren();
    CHECK(c.size == 3);
    CHECK(c.children[0].type == IRObject::Type::Int);
    CHECK(c.children[0].GetScalar<int>() == 42);
    CHECK(c.children[1].type == IRObject::Type::Float);
    CHECK(c.children[1].GetScalar<double>() == 3.14);
    CHECK(c.children[2].GetScalar<std::string>() == "test");
}

TEST_CASE("IRObject FromCorpus with int") {
    auto obj = IRObject::FromCorpus(42);
    CHECK(obj.type == IRObject::Type::Int);
    CHECK(obj.GetScalar<int>() == 42);
    
    int val = IRObject::ToCorpus<int>(obj);
    CHECK(val == 42);
}

TEST_CASE("IRObject FromCorpus with double") {
    auto obj = IRObject::FromCorpus(3.14);
    CHECK(obj.type == IRObject::Type::Float);
    CHECK(obj.GetScalar<double>() == 3.14);
    
    double val = IRObject::ToCorpus<double>(obj);
    CHECK(val == 3.14);
}

TEST_CASE("IRObject FromCorpus with string") {
    auto obj = IRObject::FromCorpus(std::string("hello"));
    CHECK(obj.GetScalar<std::string>() == "hello");
    
    std::string val = IRObject::ToCorpus<std::string>(obj);
    CHECK(val == "hello");
}

TEST_CASE("IRObject FromCorpus with vector") {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto obj = IRObject::FromCorpus(vec);
    CHECK(obj.type == IRObject::Type::Object);
    
    auto result = IRObject::ToCorpus<std::vector<int>>(obj);
    CHECK(result.size() == 5);
    CHECK(result[0] == 1);
    CHECK(result[4] == 5);
}

TEST_CASE("IRObject FromCorpus with pair") {
    auto p = std::make_pair(42, std::string("answer"));
    auto obj = IRObject::FromCorpus(p);
    CHECK(obj.type == IRObject::Type::Object);
    
    auto result = IRObject::ToCorpus<std::pair<int, std::string>>(obj);
    CHECK(result.first == 42);
    CHECK(result.second == "answer");
}

TEST_CASE("IRObject FromCorpus with tuple") {
    auto tup = std::make_tuple(1, 2.5, std::string("three"));
    auto obj = IRObject::FromCorpus(tup);
    CHECK(obj.type == IRObject::Type::Object);
    
    auto result = IRObject::ToCorpus<std::tuple<int, double, std::string>>(obj);
    CHECK(std::get<0>(result) == 1);
    CHECK(std::get<1>(result) == 2.5);
    CHECK(std::get<2>(result) == "three");
}

TEST_CASE("IRObject copy constructor") {
    IRObject obj1;
    obj1.SetScalar(42);
    
    IRObject obj2(obj1);
    CHECK(obj2.type == IRObject::Type::Int);
    CHECK(obj2.GetScalar<int>() == 42);
}

TEST_CASE("IRObject move constructor") {
    IRObject obj1;
    obj1.SetScalar(42);
    
    IRObject obj2(std::move(obj1));
    CHECK(obj2.type == IRObject::Type::Int);
    CHECK(obj2.GetScalar<int>() == 42);
}

TEST_CASE("IRObject assignment operator") {
    IRObject obj1;
    obj1.SetScalar(42);
    
    IRObject obj2;
    obj2 = obj1;
    CHECK(obj2.type == IRObject::Type::Int);
    CHECK(obj2.GetScalar<int>() == 42);
}

