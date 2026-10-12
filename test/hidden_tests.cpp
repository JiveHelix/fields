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


struct WithIndexHidden
{
    int x;
    int y;
    int z;

    std::string hideMe;
    double alsoHidden;

    static constexpr auto hidden = std::make_tuple(
        fields::IndexHide<3>{},
        fields::IndexHide<4>{});
};


DECLARE_EQUALITY_OPERATORS(WithIndexHidden);


TEMPLATE_TEST_CASE(
    "Class with hidden fields tuple HasHidden",
    "[hidden]",
    WithHidden,
    WithIndexHidden)
{
    STATIC_REQUIRE(fields::HasHidden<TestType>);
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


TEMPLATE_TEST_CASE(
    "Hidden fields are not used in json output",
    "[hidden]",
    WithHidden,
    WithIndexHidden)
{
    TestType withHidden{1, 2, 3, "I'm hidden", 3.14};

    STATIC_REQUIRE(fields::HasHidden<TestType>);

    auto asJson = fields::Unstructure<nlohmann::json>(withHidden);

    REQUIRE(asJson.count("x") == 1);
    REQUIRE(asJson.count("y") == 1);
    REQUIRE(asJson.count("z") == 1);

    REQUIRE(asJson.count("hideMe") == 0);
    REQUIRE(asJson.count("alsoHidden") == 0);

    std::cout << "withHidden: " << fields::Describe(withHidden) << std::endl;
}


TEMPLATE_TEST_CASE(
    "Hidden fields are not used from json input.",
    "[hidden]",
    WithHidden,
    WithIndexHidden)
{
    TestType withHidden{1, 2, 3, "I'm hidden", 3.14};
    auto asJson = fields::Unstructure<nlohmann::json>(withHidden);

    asJson["alsoHidden"] = -1.0;

    auto recovered = fields::Restructure<TestType>(asJson);

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


struct WithIndexedHiddenAndFields
{
    int x;
    int y;
    int z;
    std::string hideMe;
    double alsoHidden;

    static constexpr auto fields = std::make_tuple(
        fields::IndexField<0>("x"),
        fields::IndexField<1>("y"),
        fields::IndexField<2>("z"),
        fields::IndexField<3>("hideMe"),
        fields::IndexField<4>("alsoHidden"));

    static constexpr auto hidden = std::make_tuple(
        fields::IndexHide<3>{},
        fields::IndexHide<4>{});
};


struct WithMixedHiddenAndFields
{
    int x;
    int y;
    int z;
    std::string hideMe;
    double alsoHidden;

    static constexpr auto fields = std::make_tuple(
        fields::IndexField<0>("x"),
        fields::IndexField<1>("y"),
        fields::Field(&WithMixedHiddenAndFields::z, "z"),
        fields::Field(&WithMixedHiddenAndFields::hideMe, "hideMe"),
        fields::IndexField<4>("alsoHidden"));

    static constexpr auto hidden = std::make_tuple(
        fields::IndexHide<3>{},
        fields::Hide(&WithMixedHiddenAndFields::alsoHidden));
};


TEMPLATE_TEST_CASE(
    "Test Hidden json output with explicit fields.",
    "[hidden]",
    WithHiddenAndFields,
    WithIndexedHiddenAndFields,
    WithMixedHiddenAndFields)
{
    TestType withHidden{1, 2, 3, "I'm hidden", 3.14};

    STATIC_REQUIRE(fields::HasHidden<TestType>);
    STATIC_REQUIRE(fields::HasFields<TestType>);

    auto asJson = fields::Unstructure<nlohmann::json>(withHidden);

    REQUIRE(asJson.count("x") == 1);
    REQUIRE(asJson.count("y") == 1);
    REQUIRE(asJson.count("z") == 1);

    REQUIRE(asJson.count("hideMe") == 0);
    REQUIRE(asJson.count("alsoHidden") == 0);

    std::cout << "withHidden: " << fields::Describe(withHidden) << std::endl;
}


TEMPLATE_TEST_CASE(
    "Test Hidden json input with explicit fields.",
    "[hidden]",
    WithHiddenAndFields,
    WithIndexedHiddenAndFields,
    WithMixedHiddenAndFields)
{
    TestType withHidden{1, 2, 3, "I'm hidden", 3.14};
    auto asJson = fields::Unstructure<nlohmann::json>(withHidden);

    asJson["alsoHidden"] = -1.0;

    auto recovered = fields::Restructure<TestType>(asJson);

    // Normal fields are restructured.
    REQUIRE(recovered.x == 1);
    REQUIRE(recovered.y == 2);
    REQUIRE(recovered.z == 3);

    // hidden field is left untouched.
    REQUIRE(recovered.alsoHidden == 0.0);

    std::cout << "recovered: " << fields::Describe(recovered) << std::endl;
}


TEMPLATE_TEST_CASE(
    "Hidden fields are not used in comparisons",
    "[hidden]",
    WithHidden,
    WithIndexHidden)
{
    TestType withHidden{1, 2, 3, "I'm hidden", 3.14};
    TestType anotherWithHidden{1, 2, 3, "So am I", 3.14};

    STATIC_REQUIRE(fields::HasHidden<TestType>);
    REQUIRE(withHidden == anotherWithHidden);
}
