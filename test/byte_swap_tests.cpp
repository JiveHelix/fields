/**
  * @file byte_swap_tests.cpp
  *
  * @brief Test byte swapping code.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 24 Oct 2022
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#include <catch2/catch.hpp>
#include <fields/fields.h>
#include <fields/network_byte_order.h>
#include <fields/compare.h>

struct TestData
{
    int8_t a;
    int16_t b;
    int32_t c;
    int64_t d;
    uint8_t e;
    uint16_t f[4];
    uint32_t g;
    uint64_t h;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&TestData::a, "a"),
        fields::Field(&TestData::b, "b"),
        fields::Field(&TestData::c, "c"),
        fields::Field(&TestData::d, "d"),
        fields::Field(&TestData::e, "e"),
        fields::Field(&TestData::f, "f"),
        fields::Field(&TestData::g, "g"),
        fields::Field(&TestData::h, "h"));
};


DECLARE_COMPARISON_OPERATORS(TestData)


TEST_CASE("Fields byte swap", "[swap]")
{
    TestData testData{
        0x12,
        0x1234,
        0x1234ABCD,
        0x1234ABCDDCBA4321,

        0x12,
        {0x1234, 0x4321, 0xABCD, 0xBCDA},
        0x1234ABCD,
        0xABCD12344321DCBA};

    fields::HostToNetwork(testData);

    REQUIRE(testData.a == 0x12);
    REQUIRE(testData.b == 0x3412);
    REQUIRE(static_cast<unsigned int>(testData.c) == 0xCDAB3412);
    REQUIRE(testData.d == 0x2143BADCCDAB3412);
    REQUIRE(testData.e == 0x12);
    REQUIRE(testData.f[0] == 0x3412);
    REQUIRE(testData.f[1] == 0x2143);
    REQUIRE(testData.f[2] == 0xCDAB);
    REQUIRE(testData.f[3] == 0xDABC);
    REQUIRE(testData.g == 0xCDAB3412);
    REQUIRE(testData.h == 0xBADC21433412CDAB);
}


struct NetworkData
{
    int16_t a;
    int32_t b;
    int64_t c;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&NetworkData::a, "a"),
        fields::Field(&NetworkData::b, "b"),
        fields::Field(&NetworkData::c, "c"));

    // Only networkMembers will be swapped.
    static constexpr auto networkMembers = std::make_tuple(
        fields::Field(&NetworkData::b, "b"),
        fields::Field(&NetworkData::c, "c"));
};


DECLARE_COMPARISON_OPERATORS(NetworkData)


TEST_CASE("Only networkMembers are swapped", "[swap]")
{
    NetworkData networkData{
        0x1234,
        0x1234ABCD,
        0x1234ABCDDCBA4321};

    fields::HostToNetwork(networkData);

    // 'a' should remain unswapped.
    REQUIRE(networkData.a == 0x1234);

    REQUIRE(networkData.b == int32_t(0xCDAB3412));
    REQUIRE(networkData.c == 0x2143BADCCDAB3412LL);
}


TEST_CASE("All fields round trip to and from bytes", "[swap]")
{
    TestData testData{
        0x12,
        0x1234,
        0x1234ABCD,
        0x1234ABCDDCBA4321,

        0x12,
        {0x1234, 0x4321, 0xABCD, 0xBCDA},
        0x1234ABCD,
        0xABCD12344321DCBA};

    std::array<uint8_t, sizeof(TestData)> data;

    fields::ToNetworkBytes(testData, data);

    // Spot check the serialized data
    REQUIRE(data[2] == 0x12);
    REQUIRE(data[3] == 0x34);

    REQUIRE(data[4] == 0x12);
    REQUIRE(data[5] == 0x34);
    REQUIRE(data[6] == 0xAB);
    REQUIRE(data[7] == 0xCD);

    auto recovered = fields::FromNetworkBytes<TestData>(data);

    REQUIRE(recovered == testData);
}


TEST_CASE("Network fields round trip to and from bytes", "[swap]")
{
    NetworkData networkData{
        0x1234,
        0x1234ABCD,
        0x1234ABCDDCBA4321};

    std::array<uint8_t, sizeof(NetworkData)> data;

    fields::ToNetworkBytes(networkData, data);
    auto recovered = fields::FromNetworkBytes<NetworkData>(data);

    REQUIRE(recovered == networkData);
}


TEST_CASE("Array members are compared", "[swap]")
{
    TestData testData{
        0x12,
        0x1234,
        0x1234ABCD,
        0x1234ABCDDCBA4321,

        0x12,
        {0x1234, 0x4321, 0xABCD, 0xBCDA},
        0x1234ABCD,
        0xABCD12344321DCBA};

    TestData copy{testData};
    REQUIRE(copy == testData);

    testData.f[3] = 0;

    REQUIRE(copy != testData);
}
