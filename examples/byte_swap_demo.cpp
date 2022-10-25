/**
  * @file byte_swap_demo.cpp
  *
  * @brief Convert all class members to and from network byte order.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 02 Sep 2021
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/


#include <string>
#include <numeric>
#include <iostream>

#define USE_PRECISE_DIGITS
#include "fields/fields.h"

#include "fields/network_byte_order.h"

struct Position
{
    uint16_t x;
    uint16_t y;
    uint16_t z;

    constexpr static auto fields = std::make_tuple(
        fields::Field(&Position::x, "x"),
        fields::Field(&Position::y, "y"),
        fields::Field(&Position::z, "z"));
};

struct Values
{
    char identity[4];
    uint32_t wibble;
    uint16_t anArray[2][3];
    Position positions[2];

    constexpr static auto fields = std::make_tuple(
        fields::Field(&Values::identity, "identity"),
        fields::Field(&Values::wibble, "wibble"),
        fields::Field(&Values::anArray, "anArray"),
        fields::Field(&Values::positions, "positions"));

    constexpr static auto networkMembers = std::make_tuple(
        fields::Field(&Values::wibble, "wibble"),
        fields::Field(&Values::anArray, "anArray"),
        fields::Field(&Values::positions, "positions"));

    constexpr static auto fieldsTypeName = "Values";
};


int main()
{
    Values values{
        {'C', 'D', 'M', 'A'},
        0xDEADBEEF,
        {{0xCAFE, 0xBABE, 0xFEED}, {0xDEAF, 0xC0DE, 0xD00B}},
        {{0xCAFE, 0xBABE, 0xFEED}, {0xDEAF, 0xC0DE, 0xD00B}}};

    std::cout << std::hex;

    std::cout << "Host byte order:"
        << fields::DescribeColorized(values, 1) << std::endl;

    fields::HostToNetwork(values);

    std::cout << "Network byte order:"
        << fields::DescribeColorized(values, 1) << std::endl;

    fields::NetworkToHost(values);

    std::cout << "Host byte order:"
        << fields::DescribeColorized(values, 1) << std::endl;

    return 0;
}
