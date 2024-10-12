/**
  * @file default_test.cpp
  *
  * @brief Do not overwrite default constructors.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 12 Oct 2024
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/


#include <catch2/catch.hpp>
#include <fields/fields.h>
#include <nlohmann/json.hpp>


struct HasDefaultConstructor
{
    int a;
    int b;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&HasDefaultConstructor::a, "a"),
        fields::Field(&HasDefaultConstructor::b, "b"));

    HasDefaultConstructor()
        :
        a(42),
        b(31)
    {

    }
};



TEST_CASE("Structure with missing fields leaves default unchanged", "[fields]")
{
    std::string partialFields = R"({"b": 97})";

    using json = nlohmann::json;

    auto hasDefault =
        fields::Structure<HasDefaultConstructor>(json::parse(partialFields));

    REQUIRE(hasDefault.a == 42);
    REQUIRE(hasDefault.b == 97);
}


struct HasFixedSizeArray
{
    int a[4];
    int b[4];

    static constexpr auto fields = std::make_tuple(
        fields::Field(&HasFixedSizeArray::a, "a"),
        fields::Field(&HasFixedSizeArray::b, "b"));
};


TEST_CASE(
    "Structure with missing fields leaves array default initialized",
    "[fields]")
{
    std::string partialFields = R"({"b": [1, 2, 3, 4]})";

    using json = nlohmann::json;

    auto fixedSizeArray =
        fields::Structure<HasFixedSizeArray>(json::parse(partialFields));

    REQUIRE(fixedSizeArray.a[0] == 0);
    REQUIRE(fixedSizeArray.a[1] == 0);
    REQUIRE(fixedSizeArray.a[2] == 0);
    REQUIRE(fixedSizeArray.a[3] == 0);

    REQUIRE(fixedSizeArray.b[0] == 1);
    REQUIRE(fixedSizeArray.b[1] == 2);
    REQUIRE(fixedSizeArray.b[2] == 3);
    REQUIRE(fixedSizeArray.b[3] == 4);
}



