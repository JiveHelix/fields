/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include "fields/marshal.h"
#include "jive/testing/gettys_words.h"
#include "jive/testing/cast_limits.h"


TEMPLATE_TEST_CASE(
    "Round-trip full-range floating point values through Marshal",
    "[marshal]",
    float,
    double,
    long double)
{
    auto value = GENERATE(
        take(
            30,
            random(
                CastLimits<TestType>::Min(),
                CastLimits<TestType>::Max())));

    auto marshaled = fields::Marshal(value);
    TestType recovered = marshaled;
    REQUIRE(recovered == Approx(value));
}


TEMPLATE_TEST_CASE(
    "Round-trip limited-range floating point values through Marshal",
    "[marshal]",
    float,
    double,
    long double)
{
    auto value = GENERATE(
        take(
            30,
            random(
                static_cast<TestType>(-999e9),
                static_cast<TestType>(999e9))));

    auto marshaled = fields::Marshal(value);
    TestType recovered = marshaled;

    REQUIRE(recovered == Approx(value));
}


TEMPLATE_TEST_CASE(
    "Round-trip integer values through Marshal",
    "[marshal]",
    int8_t,
    uint8_t,
    int16_t,
    uint16_t,
    int32_t,
    uint32_t,
    int64_t,
    uint64_t)
{
    auto value = GENERATE(
        take(
            30,
            random(
                CastLimits<TestType>::Min(),
                CastLimits<TestType>::Max())));

    auto marshaled = fields::Marshal(value);
    TestType recovered = value;

    // Marshal is designed to preserve precision, so the == comparison should
    // pass.
    REQUIRE(recovered == value);
}


TEST_CASE("Round-trip std::string through Marshal", "[marshal]")
{
    auto wordCount = GENERATE(take(10, random(1u, 10u)));
    std::string value = RandomGettysWords().MakeWords(wordCount);
    auto marshaled = fields::Marshal(value);
    std::string recovered = marshaled;
    REQUIRE(recovered == value);
}


TEST_CASE("Round-trip booleans through Marshal", "[marshal]")
{
    bool value = true;
    auto marshaled = fields::Marshal(value);
    bool recovered = marshaled;
    REQUIRE(recovered);
    marshaled = false;
    REQUIRE(!marshaled);
}


TEMPLATE_TEST_CASE(
    "Store members in Marshal",
    "[marshal]",
    float,
    double,
    long double,
    int8_t,
    uint8_t,
    int16_t,
    uint16_t,
    int32_t,
    uint32_t,
    int64_t,
    uint64_t)
{
    auto values = GENERATE(
        take(
            10,
            chunk(
                3,
                random(
                    CastLimits<TestType>::Min(),
                    CastLimits<TestType>::Max()))));

    auto marshal = fields::Marshal();
    marshal["firstValue"] = values[0];
    marshal["secondValue"] = values[1];
    marshal["thirdValue"] = values[2];

    TestType firstValue = marshal["firstValue"];
    TestType secondValue = marshal["secondValue"];
    TestType thirdValue = marshal["thirdValue"];

    REQUIRE(firstValue == Approx(values[0]));
    REQUIRE(secondValue == Approx(values[1]));
    REQUIRE(thirdValue == Approx(values[2]));
}


TEMPLATE_TEST_CASE(
    "Store deeper levels in Marshal",
    "[marshal]",
    float,
    double,
    long double,
    int8_t,
    uint8_t,
    int16_t,
    uint16_t,
    int32_t,
    uint32_t,
    int64_t,
    uint64_t)
{
    auto value = GENERATE(
        take(
            10,
            random(
                CastLimits<TestType>::Min(),
                CastLimits<TestType>::Max())));

    auto marshal = fields::Marshal();
    marshal["levelOne"]["levelTwo"]["myValue"] = value;
    TestType recovered = marshal["levelOne"]["levelTwo"]["myValue"];
    REQUIRE(recovered == Approx(value));
}
