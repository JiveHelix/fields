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
    Bar frob;
    Foo flub;
    std::string message;

    void AfterFields()
    {
        std::cout << "Wobble has AfterFields, but it doesn't do anything."
            << std::endl;
    }

    constexpr static auto fields = std::make_tuple(
        Field(&Wobble::frob, "anyNameYouWant"),
        Field(&Wobble::flub, "Even with spaces"),
        Field(&Wobble::message, "A message for you"));

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



int main()
{
    Wobble original{
        {{13, 42000, 56.0}, {-19000, 15, 3.14}, 9.80},
        {56, 88, 3.1415926},
        "This is my message"};

    auto unstructured{fields::Unstructure<json>(original)};
    auto first = unstructured["anyNameYouWant"]["first"];
    unstructured["anyNameYouWant"].erase("first");
    unstructured["anyNameYouWant"]["primis"] = first;
    
    std::cout << unstructured["anyNameYouWant"].count("first") << std::endl;
    auto asString = unstructured.dump();

    std::cout << "unstructured: " << asString << std::endl;

    auto recoveredUnstructured = json::parse(asString);

    auto recovered = fields::Structure<Wobble>(recoveredUnstructured);

    std::cout << std::boolalpha << "recovered == original: "
        << (recovered == original) << std::endl;

    std::cout << "recovered: " << fields::Unstructure<json>(recovered).dump()
        << std::endl;

    std::cout << "Describe without indent argument:" << std::endl; 
    std::cout << fields::DescribeCompact(recovered) << std::endl;

    std::cout << "DescribeAllTypes with indent argument:" << std::endl; 
    std::cout <<
        fields::DescribeColorized<fields::DefaultColors>(recovered, 0)
        << std::endl;

    std::cout << "Change the structure color to magenta:" << std::endl;
    std::cout <<
        fields::DescribeColorized<AlteredColors>(recovered, 0) << std::endl;

    std::cout << "DescribeColorizedCompact: " << std::endl;
    std::cout <<
        fields::DescribeColorizedCompact<AlteredColors>(recovered) << std::endl;
}


