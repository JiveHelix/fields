/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2026 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include <fields/hidden.h>
#include <fields/serialize.h>
#include <fields/describe.h>
#include <fields/compare.h>


struct WithHidden
{
    int x;
    int y;
    int z;

    std::string hideMe;
    double alsoHidden;

    static constexpr auto hidden = std::make_tuple(
        fields::Hide(&WithHidden::hideMe),
        fields::Hide(&WithHidden::alsoHidden));
};


DECLARE_EQUALITY_OPERATORS(WithHidden);


TEST_CASE("Class with hidden fields tuple HasHidden", "[hidden]")
{
    STATIC_REQUIRE(fields::HasHidden<WithHidden>);
}


struct WithHiddenMember
{
    int x;
    int y;
    int z;
    std::string hidden;
};


TEST_CASE("Class with hidden member is not HasHidden.", "[hidden]")
{
    STATIC_REQUIRE(!fields::HasHidden<WithHiddenMember>);
}


struct WithHiddenTuple
{
    int x;
    int y;
    int z;

    static constexpr auto hidden = std::make_tuple(13, 42.0);
};


TEST_CASE(
    "Class with a non-hidden static constexpr tuple is not HasHidden",
    "[hidden]")
{
    STATIC_REQUIRE(!fields::HasAliases<WithHiddenTuple>);
}

#include <iostream>


TEST_CASE("Hidden fields are not used in json output", "[hidden]")
{
    WithHidden withHidden{1, 2, 3, "I'm hidden", 3.14};

    STATIC_REQUIRE(fields::HasHidden<WithHidden>);

    auto asJson = fields::Unstructure<nlohmann::json>(withHidden);

    REQUIRE(asJson.count("x") == 1);
    REQUIRE(asJson.count("y") == 1);
    REQUIRE(asJson.count("z") == 1);

    REQUIRE(asJson.count("hideMe") == 0);
    REQUIRE(asJson.count("alsoHidden") == 0);

    std::cout << "withHidden: " << fields::Describe(withHidden) << std::endl;
}


TEST_CASE("Hidden fields are not used from json input.", "[hidden]")
{
    WithHidden withHidden{1, 2, 3, "I'm hidden", 3.14};
    auto asJson = fields::Unstructure<nlohmann::json>(withHidden);

    asJson["alsoHidden"] = -1.0;

    auto recovered = fields::Restructure<WithHidden>(asJson);

    // Normal fields are restructured.
    REQUIRE(recovered.x == 1);
    REQUIRE(recovered.y == 2);
    REQUIRE(recovered.z == 3);

    // hidden field is left untouched.
    REQUIRE(recovered.alsoHidden == 0.0);

    std::cout << "recovered: " << fields::Describe(recovered) << std::endl;
}


struct WithHiddenAndFields
{
    int x;
    int y;
    int z;
    std::string hideMe;
    double alsoHidden;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&WithHiddenAndFields::x, "x"),
        fields::Field(&WithHiddenAndFields::y, "y"),
        fields::Field(&WithHiddenAndFields::z, "z"),
        fields::Field(&WithHiddenAndFields::hideMe, "hideMe"),
        fields::Field(&WithHiddenAndFields::alsoHidden, "alsoHidden"));

    static constexpr auto hidden = std::make_tuple(
        fields::Hide(&WithHiddenAndFields::hideMe),
        fields::Hide(&WithHiddenAndFields::alsoHidden));
};


TEST_CASE("Test Hidden json output with explicit fields.", "[hidden]")
{
    WithHiddenAndFields withHidden{1, 2, 3, "I'm hidden", 3.14};

    STATIC_REQUIRE(fields::HasHidden<WithHiddenAndFields>);
    STATIC_REQUIRE(fields::HasFields<WithHiddenAndFields>);

    auto asJson = fields::Unstructure<nlohmann::json>(withHidden);

    REQUIRE(asJson.count("x") == 1);
    REQUIRE(asJson.count("y") == 1);
    REQUIRE(asJson.count("z") == 1);

    REQUIRE(asJson.count("hideMe") == 0);
    REQUIRE(asJson.count("alsoHidden") == 0);

    std::cout << "withHidden: " << fields::Describe(withHidden) << std::endl;
}


TEST_CASE("Test Hidden json input with explicit fields.", "[hidden]")
{
    WithHiddenAndFields withHidden{1, 2, 3, "I'm hidden", 3.14};
    auto asJson = fields::Unstructure<nlohmann::json>(withHidden);

    asJson["alsoHidden"] = -1.0;

    auto recovered = fields::Restructure<WithHiddenAndFields>(asJson);

    // Normal fields are restructured.
    REQUIRE(recovered.x == 1);
    REQUIRE(recovered.y == 2);
    REQUIRE(recovered.z == 3);

    // hidden field is left untouched.
    REQUIRE(recovered.alsoHidden == 0.0);

    std::cout << "recovered: " << fields::Describe(recovered) << std::endl;
}


TEST_CASE("Hidden fields are not used in comparisons", "[hidden]")
{
    WithHidden withHidden{1, 2, 3, "I'm hidden", 3.14};
    WithHidden anotherWithHidden{1, 2, 3, "So am I", 3.14};

    STATIC_REQUIRE(fields::HasHidden<WithHidden>);
    REQUIRE(withHidden == anotherWithHidden);
}
