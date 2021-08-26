/**
  * @file fields_demo.cpp
  *
  * @brief A demonstration of some fields features.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 13 May 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  *
  */


#include <string>
#include <cmath>
#include <iostream>

#define ENABLE_VERBOSE_TYPES
#include "fields/fields.h"

// Silence a single warning in nlohmann/json.hpp
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop

using json = nlohmann::json;

namespace fields
{

// If we want ADL (Argument-dependent Lookup) to resolve the comparison
// operators defined in the fields namespace, we need to also add our classes
// to the fields namespace.

struct Foo
{
    int x;
    long y;
    double z;

    constexpr static auto fields = std::make_tuple(
        Field(&Foo::x, "x"),
        Field(&Foo::y, "y"),
        Field(&Foo::z, "z"));

    constexpr static auto fieldsTypeName = "Foo";

#if 0
    template<typename Colors, typename VerboseTypes>
    std::ostream & Describe(std::ostream &outputStream, int) const
    {
        return outputStream << "I am Foo";
    }
#endif

};


struct Bar
{
    Foo first;
    Foo second;
    double velocity;

    template<typename Json>
    Json Unstructure() const
    {
        Json result;

        // Use the default Unstructure for the first and second fields.
        result["first"] = fields::Unstructure<Json>(this->first);
        result["second"] = fields::Unstructure<Json>(this->second);

        // Arbitrarily store velocity as its square root.
        result["velocity"] = sqrt(this->velocity);

        return result;
    }

    template<typename Json>
    static Bar Structure(const Json &jsonValue)
    {
        Bar result;
        auto first = FindMember(std::get<0>(fields), jsonValue);

        if (first)
        {
            result.first = fields::Structure<Foo>(*first);
        }
        else
        {
            result.first = Foo{};
        }

        result.second = fields::Structure<Foo>(jsonValue.at("second"));
        double squareRoot = jsonValue.at("velocity").template get<double>();
        result.velocity = squareRoot * squareRoot;
        return result;
    }

    constexpr static auto fields = std::make_tuple(
        Field(&Bar::first, "first", "primeiro", "primis"),
        Field(&Bar::second, "second", "segundo"),
        Field(&Bar::velocity, "velocity"));

    constexpr static auto fieldsTypeName = "Bar";
};


struct Wobble
{
    uint8_t alpha[4];
    Bar frob;
    Foo flub;
    std::string message;
    std::vector<Foo> numbers;
    std::map<std::string, Foo> fooByName;

    void AfterFields()
    {
        std::cout << "Wobble has AfterFields, but it doesn't do anything."
            << std::endl;
    }

    constexpr static auto fields = std::make_tuple(
        Field(&Wobble::alpha, "alpha"),
        Field(&Wobble::frob, "anyNameYouWant"),
        Field(&Wobble::flub, "Even with spaces"),
        Field(&Wobble::message, "A message for you"),
        Field(&Wobble::numbers, "numbers"),
        Field(&Wobble::fooByName, "fooByName"));

    constexpr static auto fieldsTypeName = "Wobble";
};


} // end namespace fields

using fields::Wobble;
using fields::Bar;
using fields::Foo;

#include <iostream>

struct AlteredColors: public fields::DefaultColors
{
    static constexpr auto structure = jive::color::magenta;
};

template<typename T>
auto DescribeAltered(const T &object, int indent = -1)
{
    return fields::DescribeColorized<T, AlteredColors>(object, indent);
}

template<typename T>
auto DescribeAlteredVerbose(const T &object, int indent = -1)
{
    return fields::DescribeColorizedVerbose<T, AlteredColors>(object, indent);
}


template<typename Field>
void PrintFieldType(Field &&field)
{
    std::cout << field.name << ": " <<
        jive::GetTypeName<fields::FieldType<Field>>() << std::endl;
}


int main()
{
    Wobble original{
        {'x', 'v', 'u', 't'},
        {{13, 42000, 56.0}, {-19000, 15, 3.14}, 9.80},
        {56, 88, 3.1415926},
        "This is my message",
        {{0, 1, 2}, {117, -67, 13e-9}, {117 * 2, -67 * 2, 13e-9 * 2}},
        {}};

    original.fooByName["1st"] = {0, 1, 2};
    original.fooByName["2nd"] = {117, -67, 13e-9};
    original.fooByName["3rd"] = {117 * 2, -67 * 2, 13e-9 * 2};

    auto unstructured{fields::Unstructure<json>(original)};

    // Restructure using one of the alternate names for 'first': 'primis'
    auto first = unstructured["anyNameYouWant"]["first"];
    unstructured["anyNameYouWant"].erase("first");
    unstructured["anyNameYouWant"]["primis"] = first;

    auto asString = unstructured.dump();

    std::cout << "\nunstructured: " << asString << std::endl;

    auto recoveredUnstructured = json::parse(asString);

    auto recovered = fields::Structure<Wobble>(recoveredUnstructured);

    std::cout << std::boolalpha
              << "\nrecovered == original: " << (recovered == original)
              << std::endl;

    std::cout << "\nrecovered: " << fields::Unstructure<json>(recovered).dump()
              << std::endl;

    std::cout << "\nDescribeColorized without indent argument "
              << "(prints on one line):"
              << std::endl;

    std::cout << fields::DescribeColorized(recovered) << std::endl;

    std::cout << "\nDescribeColorizedVerbose with indent argument "
              << "(multi-line with type information):"
              << std::endl;

    std::cout << fields::DescribeColorizedVerbose(recovered, 0) << std::endl;

    std::cout << "\nDescribeColorized (no type information):" << std::endl;
    std::cout << fields::DescribeColorized(recovered, 0) << std::endl;

    std::cout << "\nChange the structure color to magenta:" << std::endl;
    std::cout << DescribeAltered(recovered, 0) << std::endl;

    std::cout << "\nDescribeAlteredVerbose: " << std::endl;
    std::cout << DescribeAlteredVerbose(recovered, 0) << std::endl;

    PrintFieldType(std::get<0>(Wobble::fields));

    return 0;
}
