/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include "fields/fields.h"
#include <optional>
#include <nlohmann/json.hpp>


// #define FIELDS_TESTS_VERBOSE


#ifdef FIELDS_TESTS_VERBOSE
template <typename... Args>
std::ostream &StreamArgs(std::ostream &outputStream, Args &&...args)
{
    return (outputStream << ... << std::forward<Args>(args)) << std::endl;
}

#define FIELDS_TEST_LOG(...) \
    StreamArgs(std::cout, ##__VA_ARGS__)

#else
#define FIELDS_TEST_LOG(...)
#endif


struct Foo
{
    int x;
    std::optional<int> y;
    std::optional<int> z;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&Foo::x, "x"),
        fields::Field(&Foo::y, "y"),
        fields::Field(&Foo::z, "z"));
};


DECLARE_EQUALITY_OPERATORS(Foo)


struct Bar
{
    float p;
    float q;
    std::optional<Foo> foo;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&Bar::p, "p"),
        fields::Field(&Bar::q, "q"),
        fields::Field(&Bar::foo, "foo"));
};


DECLARE_EQUALITY_OPERATORS(Bar)


TEST_CASE("Unstructure/Structure with optional member", "[fields]")
{
    Foo foo{};
    foo.x = 3;
    foo.z = 42;

    REQUIRE(!foo.y);
    REQUIRE(foo.z);

    FIELDS_TEST_LOG(fields::Describe(foo));
    auto asJson = fields::Unstructure<nlohmann::json>(foo);
    FIELDS_TEST_LOG("\njson:\n", asJson.dump(4), '\n');
    auto recovered = fields::Structure<Foo>(asJson);
    FIELDS_TEST_LOG(fields::Describe(recovered));

    REQUIRE(recovered.x == foo.x);
    REQUIRE(recovered.z == foo.z);
    REQUIRE(!recovered.y);
}


TEST_CASE("Unstructure/Structure with optional unset fields member", "[fields]")
{
    Bar bar{3.14f, 2.718f, {}};

    REQUIRE(!bar.foo);

    FIELDS_TEST_LOG(fields::Describe(bar));
    auto asJson = fields::Unstructure<nlohmann::json>(bar);
    FIELDS_TEST_LOG("\njson:\n", asJson.dump(4), '\n');
    auto recovered = fields::Structure<Bar>(asJson);
    FIELDS_TEST_LOG(fields::Describe(recovered));

    REQUIRE(recovered.p == bar.p);
    REQUIRE(recovered.q == bar.q);
    REQUIRE(!recovered.foo);
}


TEST_CASE("Unstructure/Structure with optional set fields member", "[fields]")
{
    Bar bar{3.14f, 2.718f, Foo{42, {}, 7}};
    REQUIRE(bar.foo);

    FIELDS_TEST_LOG(fields::Describe(bar));
    auto asJson = fields::Unstructure<nlohmann::json>(bar);
    FIELDS_TEST_LOG("\njson:\n", asJson.dump(4), '\n');
    auto recovered = fields::Structure<Bar>(asJson);
    FIELDS_TEST_LOG(fields::Describe(recovered));

    REQUIRE(recovered.p == bar.p);
    REQUIRE(recovered.q == bar.q);
    REQUIRE(!!recovered.foo);
    REQUIRE(recovered.foo == bar.foo);

    REQUIRE(recovered == bar);
}
