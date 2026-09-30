/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include <fields/aliases.h>
#include <fields/reflect.h>
#include <fields/serialize.h>
#include <fields/describe.h>
#include <optional>


struct WithAliases
{
    int x;
    int y;
    int z;
    std::string p;

    static constexpr auto aliases = std::make_tuple(
        fields::Field(&WithAliases::x, "this is x"),
        fields::Field(&WithAliases::z, "this is z"));
};


TEST_CASE("Class with aliases fields tuple HasAliases", "[aliases]")
{
    STATIC_REQUIRE(fields::HasAliases<WithAliases>);
}


struct WithAliasesMember
{
    int x;
    int y;
    int z;
    std::string aliases;
};


TEST_CASE("Class with aliases member is not HasAliases.", "[aliases]")
{
    STATIC_REQUIRE(!fields::HasAliases<WithAliasesMember>);
}


struct WithAliasesTuple
{
    int x;
    int y;
    int z;

    static constexpr auto aliases = std::make_tuple(13, 42.0);
};


TEST_CASE(
    "Class with a non-fields static constexpr tuple is not HasAliases",
    "[aliases]")
{
    STATIC_REQUIRE(!fields::HasAliases<WithAliasesTuple>);
}


TEST_CASE("Compare class member pointers.", "[fields]")
{
    static constexpr auto field1 = fields::Field(&WithAliases::y, "y");

    static constexpr auto field2 =
        fields::Field(&WithAliases::y, "some other name");

    static constexpr auto field3 = fields::Field(&WithAliases::z, "z");

    STATIC_REQUIRE(field1.member == field2.member);

    STATIC_REQUIRE(
        std::same_as<typename decltype(field1)::ClassType,
        WithAliases>);

    static constexpr WithAliases withAliases{};
    static constexpr auto members = fields::GetMemberTuple(withAliases);

    STATIC_REQUIRE(&(withAliases.*(field1.member)) == &std::get<1>(members));

    STATIC_REQUIRE(GetIsMemberField<WithAliases, 1>(field1));
    STATIC_REQUIRE(GetIsMemberField<WithAliases, 1>(field2));

    STATIC_REQUIRE(GetIsMemberField(field1, withAliases, std::get<1>(members)));
    STATIC_REQUIRE(GetIsMemberField(field2, withAliases, std::get<1>(members)));

    STATIC_REQUIRE(
        !GetIsMemberField(field3, withAliases, std::get<1>(members)));
}


TEST_CASE("Aliases are used in json output", "[aliases]")
{
    WithAliases withAliases{1, 2, 3, "p"};

    STATIC_REQUIRE(fields::HasAliases<WithAliases>);

    auto asJson = fields::Unstructure<nlohmann::json>(withAliases);

    REQUIRE(asJson.count("this is x") == 1);
    REQUIRE(asJson.count("this is z") == 1);
    REQUIRE(asJson.count("y") == 1);
}

#include <iostream>

TEST_CASE("Reflected and alias names can be used on json input.", "[aliases]")
{
    WithAliases withAliases{1, 2, 3, "p"};
    auto asJson = fields::Unstructure<nlohmann::json>(withAliases);

    asJson.erase("this is x");
    asJson["x"] = 42;

    auto recovered = fields::Restructure<WithAliases>(asJson);

    REQUIRE(recovered.x == 42);
    REQUIRE(recovered.y == 2);
    REQUIRE(recovered.z == 3);

    std::cout << "recovered: " << fields::Describe(recovered) << std::endl;
}


struct WithOtherNames
{
    int x;
    int y;
    int z;

    static constexpr auto aliases = std::make_tuple(
        fields::Field(&WithOtherNames::x, "this is x", "foo"),
        fields::Field(&WithOtherNames::z, "this is z"));
};


TEST_CASE("Alternate names can be used.", "[aliases]")
{
    WithOtherNames withOtherNames{1, 2, 3};
    auto asJson = fields::Unstructure<nlohmann::json>(withOtherNames);

    asJson.erase("this is x");
    asJson["x"] = 42;

    auto recovered = fields::Restructure<WithOtherNames>(asJson);

    REQUIRE(recovered.x == 42);
    REQUIRE(recovered.y == 2);
    REQUIRE(recovered.z == 3);

    asJson.erase("x");
    asJson["foo"] = 101;

    auto recoveredWithFoo = fields::Restructure<WithOtherNames>(asJson);

    REQUIRE(recoveredWithFoo.x == 101);
    REQUIRE(recoveredWithFoo.y == 2);
    REQUIRE(recoveredWithFoo.z == 3);

    std::cout << "recovered: " << fields::Describe(recoveredWithFoo)
        << std::endl;
}


#if 0
TODO: Create IsFieldsTuple ignores to skip reflected members on output to json or logging.

TODO: DescribeReflected needs to know about FieldNames.
#endif
