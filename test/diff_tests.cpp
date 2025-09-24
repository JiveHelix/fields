/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include "fields/diff.h"
#include "fields/fields.h"
#include <optional>
#include <nlohmann/json.hpp>


namespace difftest
{


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
    std::array<int, 3> r;
    std::array<Foo, 2> s;
    std::map<std::string, int> values;
    // Foo s[2];
    std::optional<Foo> foo;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&Bar::p, "p"),
        fields::Field(&Bar::q, "q"),
        fields::Field(&Bar::r, "r"),
        fields::Field(&Bar::s, "s"),
        fields::Field(&Bar::values, "values"),
        fields::Field(&Bar::foo, "foo"));
};


DECLARE_EQUALITY_OPERATORS(Bar)


} // end namespace difftest


TEST_CASE("Diff is sparse", "[fields]")
{
    difftest::Bar left{};
    left.p = 1.0;
    left.q = 2.0;
    left.r[0] = 13;
    left.r[1] = 14;
    left.r[2] = 15;
    left.values["forty-two"] = 42;
    left.foo = difftest::Foo{};
    left.foo->x = 42;
    left.foo->y = 43;

    // Leave foo->z unset.

    difftest::Bar right = left;

    std::cout << "left:\n" << fields::Describe(left, 1) << std::endl;
    std::cout << "right:\n" << fields::Describe(right, 1) << std::endl;

    auto shouldBeEmpty = fields::Diff<nlohmann::json>(left, right);

    if (shouldBeEmpty.has_value())
    {
        std::cout << "shouldBeEmpty:\n" << shouldBeEmpty->dump(4) << std::endl;
    }

    REQUIRE(!shouldBeEmpty);

    right.foo->x = 41;
    right.r[1] = -1;
    left.s[0].z = 19;
    right.values["forty-two"] = 54;
    right.values["fifty-four"] = 54;

    auto diff = fields::Diff<nlohmann::json>(left, right);
    REQUIRE(diff.has_value());

    std::cout << "left:\n" << fields::Describe(left, 1) << std::endl;
    std::cout << "right:\n" << fields::Describe(right, 1) << std::endl;
    std::cout << "diff:\n" << diff->dump(4) << std::endl;

    REQUIRE(left != right);
    fields::Patch(right, *diff);

    REQUIRE(left == right);

    std::cout << "left:\n" << fields::Describe(left, 1) << std::endl;
    std::cout << "right:\n" << fields::Describe(right, 1) << std::endl;
}
