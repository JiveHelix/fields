/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include <fields/reflect/reflect_member_count.h>
#include <fields/reflect/reflect.h>
#include <fields/reflect/field_index.h>
#include <optional>



TEST_CASE("Testing ReflectMemberCount", "[reflect]")
{
    struct OptionalTest
    {
        int x;
        double y;
        std::optional<short> z;
    };

    STATIC_REQUIRE(fields::ReflectMemberCount<OptionalTest>() == 3);

    struct EmptyTest
    {

    };

    STATIC_REQUIRE(fields::ReflectMemberCount<EmptyTest>() == 0);

    struct StringMemberTest
    {
        std::string z;
        int x;
        double y;
    };

    STATIC_REQUIRE(fields::ReflectMemberCount<StringMemberTest>() == 3);

    struct BaseMemberTest
    {
        std::string x;
        double y;
        int z;
    };

    struct DerivedMemberTest
    {
        BaseMemberTest onlyOneMember;
    };

    STATIC_REQUIRE(fields::ReflectMemberCount<DerivedMemberTest>() == 1);


    struct Base
    {
        std::string x;
        int y;
    };

    struct Container
    {
        std::vector<int> p;
        Base base;
        int x;
    };

    struct Derived: public Base
    {
        using Reflector = Base;
    };


    STATIC_REQUIRE(fields::ReflectMemberCount<Base>() == 2);

    STATIC_REQUIRE(
        fields::ReflectMemberCount<fields::ReflectorType<Derived>>() == 2);

    STATIC_REQUIRE(fields::ReflectMemberCount<Container>() == 3);

    struct B { std::array<int,2> a; int k; };
    STATIC_REQUIRE(fields::ReflectMemberCount<B>() == 2);
}


namespace name_tests
{


template<typename T>
struct HasT
{
    T value;
};


struct Names
{
    int foo;
    std::vector<float> bar;
    HasT<double> wibble;
    std::vector<HasT<double>> wobble;
};


} // end namespace name_tests


TEST_CASE("Testing member names", "[reflect]")
{
    using Reflected = fields::Reflect<name_tests::Names>;
    REQUIRE(Reflected::name<0> == "foo");
    REQUIRE(Reflected::name<1> == "bar");
    REQUIRE(Reflected::name<2> == "wibble");
    REQUIRE(Reflected::name<3> == "wobble");
}


TEST_CASE("FieldIndex returns correct value", "[reflect]")
{
    STATIC_REQUIRE(fields::FieldIndex<"foo", name_tests::Names>() == 0);
    STATIC_REQUIRE(fields::FieldIndex<"bar", name_tests::Names>() == 1);
    STATIC_REQUIRE(fields::FieldIndex<"wibble", name_tests::Names>() == 2);
    STATIC_REQUIRE(fields::FieldIndex<"wobble", name_tests::Names>() == 3);
}
