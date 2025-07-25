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
#include <iomanip>


#define USE_PRECISE_DIGITS
#include "fields/core.h"

#ifdef __clang__
// Silence a single warning in nlohmann/json.hpp
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
#endif
#include <nlohmann/json.hpp>
#ifdef __clang__
#pragma GCC diagnostic pop
#endif

using json = nlohmann::json;


#include <fields/describe.h>


enum class Python: uint8_t
{
    chapman,
    cleese,
    gilliam,
    idle,
    jones,
    palin
};


inline
std::ostream & DoDescribe(
    std::ostream &outputStream,
    Python python,
    const fields::Style &,
    int)
{
    switch (python)
    {
        case Python::chapman:
            return outputStream << "Graham Chapman";

        case Python::cleese:
            return outputStream << "John Cleese";

        case Python::gilliam:
            return outputStream << "Terry Gilliam";

        case Python::idle:
            return outputStream << "Eric Idle";

        case Python::jones:
            return outputStream << "Terry Jones";

        case Python::palin:
            return outputStream << "Michael Palin";

        default:
            throw std::runtime_error("Unkown Python");
    }
}


struct Groot
{
    int x;
    long y;
    double z;

#if 0
    // Should use Reflection instead of explicit fields.
    constexpr static auto fields = std::make_tuple(
        fields::Field(&Groot::x, "x"),
        fields::Field(&Groot::y, "y"),
        fields::Field(&Groot::z, "z"));
#endif

    constexpr static auto fieldsTypeName = "Groot";

    static constexpr size_t precision = 3;
};


DECLARE_OUTPUT_STREAM_OPERATOR(Groot)


struct Bar
{
    Groot first;
    Groot second;
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
            result.first = fields::Structure<Groot>(*first);
        }
        else
        {
            result.first = Groot{};
        }

        result.second = fields::Structure<Groot>(jsonValue.at("second"));
        double squareRoot = jsonValue.at("velocity").template get<double>();
        result.velocity = squareRoot * squareRoot;
        return result;
    }

    constexpr static auto fields = std::make_tuple(
        fields::Field(&Bar::first, "first", "primeiro", "primis"),
        fields::Field(&Bar::second, "second", "segundo"),
        fields::Field(&Bar::velocity, "velocity"));

    constexpr static auto fieldsTypeName = "Bar";
};


DECLARE_OUTPUT_STREAM_OPERATOR(Bar)


struct Wobble
{
    uint8_t alpha[2][4];
    Bar frob;
    Groot flub[2][2];
    std::string message;
    std::vector<Groot> numbers;
    std::map<std::string, Groot> fooByName;

    void AfterFields()
    {
        std::cout << "Wobble has AfterFields, but it doesn't do anything."
            << std::endl;
    }

    constexpr static auto fields = std::make_tuple(
        fields::Field(&Wobble::alpha, "alpha"),
        fields::Field(&Wobble::frob, "any name you want"),
        fields::Field(&Wobble::flub, "flub"),
        fields::Field(&Wobble::message, "message"),
        fields::Field(&Wobble::numbers, "numbers"),
        fields::Field(&Wobble::fooByName, "fooByName"));

    constexpr static auto fieldsTypeName = "Wobble";
};


DECLARE_OUTPUT_STREAM_OPERATOR(Wobble)

#include <fields/compare.h>

DECLARE_COMPARISON_OPERATORS(Groot)
DECLARE_COMPARISON_OPERATORS(Bar)
DECLARE_COMPARISON_OPERATORS(Wobble)



struct AlteredColors: public fields::DefaultColors
{
    static constexpr auto structure = jive::color::magenta;
};

template<typename T>
auto DescribeAltered(const T &object, int indent = -1)
{
    return fields::Describe<T, AlteredColors>(object, indent);
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


struct Rocket
{
    Python python;
    int x;
    long y;
    std::optional<double> z;

    constexpr static auto fields = std::make_tuple(
        fields::Field(&Rocket::python, "python"),
        fields::Field(&Rocket::x, "x"),
        fields::Field(&Rocket::y, "y"),
        fields::Field(&Rocket::z, "z"));

    constexpr static auto fieldsTypeName = "Rocket";
    static constexpr size_t precision = 3;
};


using OptionalZ = std::optional<double>;
static_assert(jive::IsOptional<OptionalZ>);


int main()
{
    Wobble original{
        {{'x', 'v', 'u', 't'}, {'s', 'r', 'q', 'p'}},
        {{13, 42000, 56.0}, {-19000, 15, 3.14}, 9.80},
        {
            {{56, 88, 3.1415926}, {57, 89, 4.1415926}},
            {{58, 90, 5.1415926}, {59, 60, 6.1415926}}},
        "This is my message",
        {{0, 1, 2}, {117, -67, 13e-9}, {117 * 2, -67 * 2, 13e-9 * 2}},
        {}};

    original.fooByName["1st"] = {0, 1, 2};
    original.fooByName["2nd"] = {117, -67, 13e-9};
    original.fooByName["3rd"] = {117 * 2, -67 * 2, 13e-9 * 2};

    auto unstructured = fields::Unstructure<json>(original);

    // Restructure using one of the alternate names for 'first': 'primis'
    auto first = unstructured["any name you want"]["first"];
    unstructured["any name you want"].erase("first");
    unstructured["any name you want"]["primis"] = first;

    std::cout << "\nunstructured:\n" << std::setw(4) << unstructured
        << std::endl;

    auto asString = unstructured.dump();

    auto recoveredUnstructured = json::parse(asString);

    std::cout << "recoveredUnstructured:\n" << std::setw(4)
        << recoveredUnstructured << std::endl;

    auto recovered = fields::Structure<Wobble>(recoveredUnstructured);


    std::cout << "\nDescribeColorizedVerbose with indent argument "
              << "(multi-line with type information):"
              << std::endl;

    std::cout << fields::DescribeColorizedVerbose(recovered, 0) << std::endl;


    std::cout << std::boolalpha
              << "\nrecovered == original: " << (recovered == original)
              << std::endl;

    recovered.flub[1][0].z = 5.140;

    std::cout << std::boolalpha
              << "\nrecovered == original: " << (recovered == original)
              << std::endl;

    std::cout << "\nDescribe without indent argument "
              << "(prints on one line):"
              << std::endl;

    std::cout << fields::Describe(recovered) << std::endl;

    std::cout << "\nDescribe (no type information):" << std::endl;
    std::cout << fields::Describe(recovered, 0) << std::endl;

    std::cout << "\nChange the structure color to magenta:" << std::endl;
    std::cout << DescribeAltered(recovered, 0) << std::endl;

    std::cout << "\nDescribeAlteredVerbose: " << std::endl;
    std::cout << DescribeAlteredVerbose(recovered, 0) << std::endl;

    Rocket rocket{Python::gilliam, 1, 2, {}};
    std::cout << fields::Describe(rocket) << std::endl;

    rocket.z = 42.0;
    std::cout << fields::Describe(rocket) << std::endl;

    return 0;
}
