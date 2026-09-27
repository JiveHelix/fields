#pragma once


#include "get_member_count.h"


namespace fields
{


template<typename T, size_t memberCount = GetMemberCount<T>()>
    requires (memberCount <= maximumReflectCount)
constexpr decltype(auto) GetMemberTuple(T &&t)
{
    if constexpr (memberCount == 0)
    {
        return std::tuple{};
    }
    else if constexpr (memberCount == 1)
    {
        auto && [_1] = t;

        return std::tie(_1);
    }
    else if constexpr (memberCount == 2)
    {
        auto && [_1, _2] = t;

        return std::tie(_1, _2);
    }
    else if constexpr (memberCount == 3)
    {
        auto && [_1, _2, _3] = t;

        return std::tie(_1, _2, _3);
    }
    else if constexpr (memberCount == 4)
    {
        auto && [_1, _2, _3, _4] = t;

        return std::tie(_1, _2, _3, _4);
    }
    else if constexpr (memberCount == 5)
    {
        auto && [_1, _2, _3, _4, _5] = t;

        return std::tie(_1, _2, _3, _4, _5);
    }
    else if constexpr (memberCount == 6)
    {
        auto && [_1, _2, _3, _4, _5, _6] = t;

        return std::tie(_1, _2, _3, _4, _5, _6);
    }
    else if constexpr (memberCount == 7)
    {
        auto && [_1, _2, _3, _4, _5, _6, _7] = t;

        return std::tie(_1, _2, _3, _4, _5, _6, _7);
    }
    else if constexpr (memberCount == 8)
    {
        auto && [_1, _2, _3, _4, _5, _6, _7, _8] = t;

        return std::tie(_1, _2, _3, _4, _5, _6, _7, _8);
    }
    else if constexpr (memberCount == 9)
    {
        auto && [_1, _2, _3, _4, _5, _6, _7, _8, _9] = t;

        return std::tie(_1, _2, _3, _4, _5, _6, _7, _8, _9);
    }
    else if constexpr (memberCount == 10)
    {
        auto && [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10] = t;

        return std::tie(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10);
    }
    else if constexpr (memberCount == 11)
    {
        auto && [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11] = t;

        return std::tie(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11);
    }
    else if constexpr (memberCount == 12)
    {
        auto && [_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12] = t;

        return std::tie(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12);
    }
    else if constexpr (memberCount == 13)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13);
    }
    else if constexpr (memberCount == 14)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14);
    }
    else if constexpr (memberCount == 15)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15);
    }
    else if constexpr (memberCount == 16)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16);
    }
    else if constexpr (memberCount == 17)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17);
    }
    else if constexpr (memberCount == 18)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18);
    }
    else if constexpr (memberCount == 19)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19);
    }
    else if constexpr (memberCount == 20)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20);
    }
    else if constexpr (memberCount == 21)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20, _21] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20, _21);
    }
    else if constexpr (memberCount == 22)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20, _21, _22] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20, _21, _22);
    }
    else if constexpr (memberCount == 23)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20, _21, _22, _23] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20, _21, _22, _23);
    }
    else if constexpr (memberCount == 24)
    {
        auto && [
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20, _21, _22, _23, _24] = t;

        return std::tie(
            _1, _2, _3, _4, _5, _6, _7, _8,
            _9, _10, _11, _12, _13, _14, _15, _16,
            _17, _18, _19, _20, _21, _22, _23, _24);
    }
}


} // end namespace fields
