/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include "fields/fields.h"
#include <optional>
#include <nlohmann/json.hpp>


// #define FIELDS_TESTS_VERBOSE


#ifdef FIELDS_TESTS_VERBOSE
template <typename... Args>
std::ostream & StreamArgs(std::ostream &outputStream, Args &&...args)
{
    return (outputStream << ... << std::forward<Args>(args)) << std::endl;
}

#define FIELDS_TEST_LOG(...) \
    StreamArgs(std::cout, __VA_ARGS__)

#else
#define FIELDS_TEST_LOG(...)
#endif


struct Foo
{
    int x;
    std::optional<int> y;
    std::optional<int> z;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&Foo::x, "x"),
        fields::Field(&Foo::y, "y"),
        fields::Field(&Foo::z, "z"));
};


DECLARE_EQUALITY_OPERATORS(Foo)


struct Bar
{
    float p;
    float q;
    std::optional<Foo> foo;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&Bar::p, "p"),
        fields::Field(&Bar::q, "q"),
        fields::Field(&Bar::foo, "foo"));
};


DECLARE_EQUALITY_OPERATORS(Bar)


TEST_CASE("Unstructure/Structure with optional member", "[fields]")
{
    Foo foo{};
    foo.x = 3;
    foo.z = 42;

    REQUIRE(!foo.y);
    REQUIRE(foo.z);

    FIELDS_TEST_LOG(fields::Describe(foo));
    auto asJson = fields::Unstructure<nlohmann::json>(foo);
    FIELDS_TEST_LOG("\njson:\n", asJson.dump(4), '\n');
    auto recovered = fields::Structure<Foo>(asJson);
    FIELDS_TEST_LOG(fields::Describe(recovered));

    REQUIRE(recovered.x == foo.x);
    REQUIRE(recovered.z == foo.z);
    REQUIRE(!recovered.y);
}


TEST_CASE("Unstructure/Structure with optional unset fields member", "[fields]")
{
    Bar bar{3.14f, 2.718f, {}};

    REQUIRE(!bar.foo);

    FIELDS_TEST_LOG(fields::Describe(bar));
    auto asJson = fields::Unstructure<nlohmann::json>(bar);
    FIELDS_TEST_LOG("\njson:\n", asJson.dump(4), '\n');
    auto recovered = fields::Structure<Bar>(asJson);
    FIELDS_TEST_LOG(fields::Describe(recovered));

    REQUIRE(recovered.p == bar.p);
    REQUIRE(recovered.q == bar.q);
    REQUIRE(!recovered.foo);
}


TEST_CASE("Unstructure/Structure with optional set fields member", "[fields]")
{
    Bar bar{3.14f, 2.718f, Foo{42, {}, 7}};
    REQUIRE(bar.foo);

    FIELDS_TEST_LOG(fields::Describe(bar));
    auto asJson = fields::Unstructure<nlohmann::json>(bar);
    FIELDS_TEST_LOG("\njson:\n", asJson.dump(4), '\n');
    auto recovered = fields::Structure<Bar>(asJson);
    FIELDS_TEST_LOG(fields::Describe(recovered));

    REQUIRE(recovered.p == bar.p);
    REQUIRE(recovered.q == bar.q);
    REQUIRE(!!recovered.foo);
    REQUIRE(recovered.foo == bar.foo);

    REQUIRE(recovered == bar);
}


enum class Python: uint8_t
{
    chapman,
    cleese,
    gilliam,
    idle,
    jones,
    palin
};


std::map<Python, std::string_view> pythonNamesById{
    {Python::chapman, "Graham Chapman"},
    {Python::cleese, "John Cleese"},
    {Python::gilliam, "Terry Gilliam"},
    {Python::idle, "Eric Idle"},
    {Python::jones, "Terry Jones"},
    {Python::palin, "Michael Palin"}};


std::unordered_map<std::string_view, Python> GetIdByPythonName()
{
    std::unordered_map<std::string_view, Python> result;

    for (auto [key, name]: pythonNamesById)
    {
        result[name] = key;
    }

    return result;
}


Python ToValue(fields::Tag<Python>, std::string_view asString)
{
    static const auto idByPythonName = GetIdByPythonName();

    return idByPythonName.at(asString);
}

std::string ToString(Python python)
{
    return std::string(pythonNamesById.at(python));
}


std::ostream & DoDescribe(
    std::ostream &outputStream,
    Python python,
    const fields::Style &,
    int)
{
    return outputStream << pythonNamesById.at(python);
}


struct ComedyTroupe
{
    std::string movieTitle;
    Python arthurKingOfTheBritons;
    Python sirLancelotTheBrave;
    Python patsyArthursServant;
    Python sirRobinTheNotQuiteSoBraveAsSirLancelot;
    Python sirVedevereTheWise;
    Python sirGalahadThePure;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&ComedyTroupe::movieTitle, "movieTitle"),
        fields::Field(
            &ComedyTroupe::arthurKingOfTheBritons,
            "arthurKingOfTheBritons"),
        fields::Field(
            &ComedyTroupe::sirLancelotTheBrave,
            "sirLancelotTheBrave"),
        fields::Field(
            &ComedyTroupe::patsyArthursServant,
            "patsyArthursServant"),
        fields::Field(
            &ComedyTroupe::sirRobinTheNotQuiteSoBraveAsSirLancelot,
            "sirRobinTheNotQuiteSoBraveAsSirLancelot"),
        fields::Field(&ComedyTroupe::sirVedevereTheWise, "sirVedevereTheWise"),
        fields::Field(&ComedyTroupe::sirGalahadThePure, "sirGalahadThePure"));
};


DECLARE_EQUALITY_OPERATORS(ComedyTroupe)


TEST_CASE("Unstructure/Structure a comedy troupe", "[fields]")
{
    static_assert(fields::HasToValue<Python>);
    static_assert(fields::HasToString<Python>);

    ComedyTroupe troupe
    {
        .movieTitle = "Monty Python and the Holy Grail",
        .arthurKingOfTheBritons = Python::chapman,
        .sirLancelotTheBrave = Python::cleese,
        .patsyArthursServant = Python::gilliam,
        .sirRobinTheNotQuiteSoBraveAsSirLancelot = Python::idle,
        .sirVedevereTheWise = Python::jones,
        .sirGalahadThePure = Python::palin
    };

    FIELDS_TEST_LOG(fields::DescribeColorized(troupe, 1));
    auto asJson = fields::Unstructure<nlohmann::json>(troupe);
    FIELDS_TEST_LOG("\njson:\n", asJson.dump(4), '\n');
    auto recovered = fields::Structure<ComedyTroupe>(asJson);
    FIELDS_TEST_LOG(fields::DescribeColorized(recovered, 1));

    REQUIRE(recovered == troupe);
}
