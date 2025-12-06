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
#include <fields/reflect/member_names.h>

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
    std::array<Position, 2> positions;

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


struct Fubar
{
    int eloise;
    std::vector<float> ruth;
    double euler;
};

using Test0 = fields::MemberName_<0, Fubar>;
using Test1 = fields::MemberName_<1, Fubar>;
using Test2 = fields::MemberName_<2, Fubar>;


int TestNarrowing(double value)
{
    return static_cast<int>(value);
}

double TestConversion(int value)
{
    return value;
}


int main()
{
    Values values{
        {'C', 'D', 'M', 'A'},
        0xDEADBEEF,
        {{0xCAFE, 0xBABE, 0xFEED}, {0xDEAF, 0xC0DE, 0xD00B}},
        {{{0xCAFE, 0xBABE, 0xFEED}, {0xDEAF, 0xC0DE, 0xD00B}}}};

    std::cout << std::hex;

    std::cout << "Host byte order:"
        << fields::Describe(values, 1) << std::endl;

    fields::HostToNetwork(values);

    std::cout << "Network byte order:"
        << fields::Describe(values, 1) << std::endl;

    fields::NetworkToHost(values);

    std::cout << "Host byte order:"
        << fields::Describe(values, 1) << std::endl;

    std::cout << std::dec;
    std::cout << fields::PrettyField::pretty << std::endl;
    std::cout << Test0::pretty << std::endl;
    std::cout << Test1::pretty << std::endl;
    std::cout << Test2::pretty << std::endl;
    std::cout << Test0::nameOffset << std::endl;
    std::cout << Test1::nameOffset << std::endl;
    std::cout << Test2::nameOffset << std::endl;

    std::cout << Test0::name << std::endl;
    std::cout << Test1::name << std::endl;
    std::cout << Test2::name << std::endl;

    std::cout << jive::detail::TypeName<fields::Pointer<int>>()
            << std::endl;

    std::cout << jive::detail::TypeName<fields::Pointer<std::vector<float>>>()
            << std::endl;

    std::cout << jive::detail::TypeName<fields::Pointer<double>>()
            << std::endl;

    std::cout << jive::detail::TypeName<int>()
            << std::endl;

    std::cout << jive::detail::TypeName<std::vector<float>>()
            << std::endl;

    std::cout << jive::detail::TypeName<double>()
            << std::endl;

    std::cout << TestNarrowing(42.0) << std::endl;
    std::cout << TestConversion(-42) << std::endl;

    return 0;
}
