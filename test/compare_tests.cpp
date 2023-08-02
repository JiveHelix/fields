/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include "fields/marshal.h"
#include "fields/core.h"
#include "fields/compare.h"


struct CompareMe
{
    float x;
    float y;
    float z;

    static constexpr ssize_t precision = 4;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&CompareMe::x, "x"),
        fields::Field(&CompareMe::y, "y"),
        fields::Field(&CompareMe::z, "z"));
};


TEST_CASE("Compare almost equal", "[compare]")
{
    auto left = CompareMe{2.0f, 3.0f, 40.0f};
    auto right = CompareMe{2.0f, 3.0f, 40.01};

    REQUIRE(
        fields::ComparisonTuple(left, CompareMe::fields)
            != fields::ComparisonTuple(right, CompareMe::fields));

    right.z = 40.001;

    REQUIRE(
        fields::ComparisonTuple(left, CompareMe::fields)
            == fields::ComparisonTuple(right, CompareMe::fields));
}
