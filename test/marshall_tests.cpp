/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include "fields/marshall.h"
#include "jive/testing/gettys_words.h"
#include "jive/testing/cast_limits.h"


TEMPLATE_TEST_CASE(
    "Round-trip full-range floating point values through Marshall",
    "[marshall]",
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

    auto marshalled = fields::Marshall(value);
    TestType recovered = marshalled;
    REQUIRE(recovered == Approx(value));
}


TEMPLATE_TEST_CASE(
    "Round-trip limited-range floating point values through Marshall",
    "[marshall]",
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

    auto marshalled = fields::Marshall(value);
    TestType recovered = marshalled;

    REQUIRE(recovered == Approx(value));
}


TEMPLATE_TEST_CASE(
    "Round-trip integer values through Marshall",
    "[marshall]",
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

    auto marshalled = fields::Marshall(value);
    TestType recovered = value;

    // Marshall is designed to preserve precision, so the == comparison should
    // pass.
    REQUIRE(recovered == value);
}


TEST_CASE("Round-trip std::string through Marshall", "[marshall]")
{
    auto wordCount = GENERATE(take(10, random(1u, 10u)));
    std::string value = RandomGettysWords().MakeWords(wordCount);
    auto marshalled = fields::Marshall(value);
    std::string recovered = marshalled;
    REQUIRE(recovered == value);
}


TEST_CASE("Round-trip booleans through Marshall", "[marshall]")
{
    bool value = true;
    auto marshalled = fields::Marshall(value);
    bool recovered = marshalled;
    REQUIRE(recovered);
    marshalled = false;
    REQUIRE(!marshalled);
}


TEMPLATE_TEST_CASE(
    "Store members in Marshall",
    "[marshall]",
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

    auto marshall = fields::Marshall();
    marshall["firstValue"] = values[0];
    marshall["secondValue"] = values[1];
    marshall["thirdValue"] = values[2];

    TestType firstValue = marshall["firstValue"];
    TestType secondValue = marshall["secondValue"];
    TestType thirdValue = marshall["thirdValue"];

    REQUIRE(firstValue == Approx(values[0]));
    REQUIRE(secondValue == Approx(values[1]));
    REQUIRE(thirdValue == Approx(values[2]));
}


TEMPLATE_TEST_CASE(
    "Store deeper levels in Marshall",
    "[marshall]",
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

    auto marshall = fields::Marshall();
    marshall["levelOne"]["levelTwo"]["myValue"] = value;
    TestType recovered = marshall["levelOne"]["levelTwo"]["myValue"];
    REQUIRE(recovered == Approx(value));
}
