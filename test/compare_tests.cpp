/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include "fields/marshal.h"
#include "fields/core.h"
#include "fields/compare.h"


struct ImplicitPrecision
{
    float a;
    float b;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&ImplicitPrecision::a, "a"),
        fields::Field(&ImplicitPrecision::b, "b"));
};


DECLARE_EQUALITY_OPERATORS(ImplicitPrecision)
DECLARE_OUTPUT_STREAM_OPERATOR(ImplicitPrecision)


struct CompareMe
{
    float x;
    float y;
    float z;
    ImplicitPrecision foo;

    static constexpr ssize_t precision = 4;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&CompareMe::x, "x"),
        fields::Field(&CompareMe::y, "y"),
        fields::Field(&CompareMe::z, "z"),
        fields::Field(&CompareMe::foo, "foo"));
};

DECLARE_OUTPUT_STREAM_OPERATOR(CompareMe)


TEST_CASE("Compare almost equal", "[compare]")
{
    auto left = CompareMe{2.0f, 3.0f, 40.0f, {1.0f, 1.0f}};
    auto right = CompareMe{2.0f, 3.0f, 40.01f, {1.0f, 1.0001f}};

    REQUIRE(
        fields::ComparisonTuple(left) != fields::ComparisonTuple(right));

    right.z = 40.001f;

    REQUIRE(
        fields::ComparisonTuple(left) == fields::ComparisonTuple(right));

    right.foo.b = 1.001f;

    REQUIRE(
        fields::ComparisonTuple(left) != fields::ComparisonTuple(right));
}


struct CompareWithOptional
{
    float x;
    float y;
    float z;
    std::optional<ImplicitPrecision> foo;

    static constexpr ssize_t precision = 4;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&CompareWithOptional::x, "x"),
        fields::Field(&CompareWithOptional::y, "y"),
        fields::Field(&CompareWithOptional::z, "z"),
        fields::Field(&CompareWithOptional::foo, "foo"));
};


DECLARE_EQUALITY_OPERATORS(CompareWithOptional)
DECLARE_OUTPUT_STREAM_OPERATOR(CompareWithOptional)


TEST_CASE("Compare almost equal with optional", "[compare]")
{
    auto left =
        CompareWithOptional{2.0f, 3.0f, 40.0f, ImplicitPrecision{1.0f, 1.0f}};

    auto right =
        CompareWithOptional{
            2.0f,
            3.0f,
            40.01f,
            ImplicitPrecision{1.0f, 1.0001f}};

    REQUIRE(
        fields::ComparisonTuple(left) != fields::ComparisonTuple(right));

    right.z = 40.001f;

    if (left != right)
    {
        std::cout << fields::Describe(left, 1) << std::endl;
        std::cout << fields::Describe(right, 1) << std::endl;
    }

    REQUIRE(
        fields::ComparisonTuple(left) == fields::ComparisonTuple(right));

    right.foo->b = 1.001f;

    REQUIRE(
        fields::ComparisonTuple(left) != fields::ComparisonTuple(right));

    left.foo = {};
    right.foo = {};

    REQUIRE(
        fields::ComparisonTuple(left) == fields::ComparisonTuple(right));
}


struct ExplicitPrecision
{
    float a;
    float b;

    static constexpr ssize_t precision = 6;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&ExplicitPrecision::a, "a"),
        fields::Field(&ExplicitPrecision::b, "b"));
};


DECLARE_EQUALITY_OPERATORS(ExplicitPrecision)


struct CompareMeToo
{
    float x;
    float y;
    float z;
    ExplicitPrecision foo;

    static constexpr ssize_t precision = 4;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&CompareMeToo::x, "x"),
        fields::Field(&CompareMeToo::y, "y"),
        fields::Field(&CompareMeToo::z, "z"),
        fields::Field(&CompareMeToo::foo, "foo"));
};


TEST_CASE("Compare explicit precision", "[compare]")
{
    auto left = CompareMeToo{2.0f, 3.0f, 40.0f, {1.0f, 1.0f}};
    auto right = CompareMeToo{2.0f, 3.0f, 40.01f, {1.0f, 1.0001f}};

    REQUIRE(
        fields::ComparisonTuple(left) != fields::ComparisonTuple(right));

    right.z = 40.001f;

    REQUIRE(
        fields::ComparisonTuple(left) != fields::ComparisonTuple(right));

    right.foo.b = 1.000001f;

    REQUIRE(
        fields::ComparisonTuple(left) == fields::ComparisonTuple(right));
}
