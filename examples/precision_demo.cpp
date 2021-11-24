/**
  * @file precision_demo.cpp
  * 
  * @brief Floating-point comparison demonstration.
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
#include "fields/comparisons.h"

#include "jive/begin.h"


struct Position
{
    double x;
    double y;
    double z;

    constexpr static auto fields = std::make_tuple(
        fields::Field(&Position::x, "x"),
        fields::Field(&Position::y, "y"),
        fields::Field(&Position::z, "z"));

    constexpr static auto fieldsTypeName = "Position";

    static constexpr size_t precision = 2;
};


struct AnArrayOfValues
{
    double values[2][3];

    constexpr static auto fields = std::make_tuple(
        fields::Field(&AnArrayOfValues::values, "values"));

    static constexpr size_t precision = 3;
};

DECLARE_OUTPUT_STREAM_OPERATOR(Position);
DECLARE_OUTPUT_STREAM_OPERATOR(AnArrayOfValues);


int main()
{
    Position p{4.2, 6.342, -1.36};
    Position q{4.2, 6.349, -1.36};

    std::cout << std::boolalpha;

    std::cout << p << " < " << q << ": " << (p < q) << std::endl;
    std::cout << p << " == " << q << ": " << (p == q) << std::endl;

    AnArrayOfValues t{};
    AnArrayOfValues u{};
    
    std::iota(
        jive::Begin(t.values),
        jive::End(t.values),
        1.0);
    
    u = t;

    // Should still compare equal
    u.values[1][1] += 0.0005;
    
    std::cout << "Expect equal:" << std::endl;
    std::cout << t << " == " << u << ": " << (t == u) << std::endl;

    // Should compare false
    u.values[1][1] += 0.00050000001;

    std::cout << "Expect not equal:" << std::endl;
    std::cout << t << " == " << u << ": " << (t == u) << std::endl;

    return 0;
}
