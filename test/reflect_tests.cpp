/**
  * @author Jive Helix (jivehelix@gmail.com)
  * @copyright 2020 Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#include <catch2/catch.hpp>
#include <fields/reflect/get_member_count.h>
#include <fields/reflect/reflect.h>
#include <optional>



TEST_CASE("Testing GetMemberCount", "[fields]")
{
    struct OptionalTest
    {
        int x;
        double y;
        std::optional<short> z;
    };

    STATIC_REQUIRE(fields::GetMemberCount<OptionalTest>() == 3);

    struct EmptyTest
    {

    };

    STATIC_REQUIRE(fields::GetMemberCount<EmptyTest>() == 0);

    struct StringMemberTest
    {
        std::string z;
        int x;
        double y;
    };

    STATIC_REQUIRE(fields::GetMemberCount<StringMemberTest>() == 3);

    struct BaseMemberTest
    {
        std::string x;
        double y;
        int z;
    };

    struct DerivedMemberTest: public BaseMemberTest
    {

    };

    STATIC_REQUIRE(fields::GetMemberCount<DerivedMemberTest>() == 3);


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

    };


    STATIC_REQUIRE(fields::GetMemberCount<Base>() == 2);
    STATIC_REQUIRE(fields::GetMemberCount<Derived>() == 2);
    STATIC_REQUIRE(fields::GetMemberCount<Container>() == 3);

    struct B { std::array<int,2> a; int k; };
    STATIC_REQUIRE(fields::GetMemberCount<B>() == 2);
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


TEST_CASE("Testing member names", "[fields]")
{
    using Reflected = fields::Reflect<name_tests::Names>;
    REQUIRE(Reflected::name<0> == "foo");
    REQUIRE(Reflected::name<1> == "bar");
    REQUIRE(Reflected::name<2> == "wibble");
    REQUIRE(Reflected::name<3> == "wobble");
}
