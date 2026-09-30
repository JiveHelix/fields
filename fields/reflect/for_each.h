#pragma once

#include <fields/reflect/reflect.h>
#include <fields/aliases.h>


#include <iostream>

namespace fields
{


template<size_t Index, typename T>
    requires CanReflect<T>
decltype(auto) GetMember(T &&t)
{
    return std::get<Index>(
        GetMemberTuple(ForwardReflector<T>(std::forward<T>(t))));
}


template<size_t I, typename Tuple>
decltype(auto) ForwardGet(Tuple &&tuple)
{
    return std::get<I>(std::forward<Tuple>(tuple));
}


template<typename T, typename Function, size_t... Is>
    requires CanReflect<T>
void ForEachImpl(T &&t, Function &&function, std::index_sequence<Is...>)
{
    auto &&members = GetMemberTuple(ForwardReflector<T>(std::forward<T>(t)));
    using Members = decltype(members);

    static constexpr auto names = MemberNames<ReflectorType<T>>;

    constexpr bool acceptsNames = (
        std::is_invocable_v
        <
            Function &,
            decltype(FieldNames<>(std::get<Is>(names))),

            decltype(ForwardGet<Is>(
                std::forward<Members>(members)))
        > && ...
    );

    constexpr bool acceptsFieldNames = HasAliases<T> && acceptsNames;

    if constexpr (acceptsFieldNames)
    {
        (function(
            GetFieldNames<T, Is>(),
            ForwardGet<Is>(std::forward<Members>(members))), ...);
    }
    else if constexpr (acceptsNames)
    {
        (function(
            FieldNames<>(std::get<Is>(names)),
            ForwardGet<Is>(std::forward<Members>(members))), ...);
    }
    else
    {
        // Function only expects this member.
        (function(
            ForwardGet<Is>(std::forward<Members>(members))), ...);
    }
}


template<typename T, typename Function>
    requires CanReflect<T>
void ForEach(T &&t, Function &&function)
{
    static constexpr auto count = GetMemberCount<ReflectorType<T>>();

    ForEachImpl(
        ForwardReflector<T>(std::forward<T>(t)),
        std::forward<Function>(function),
        std::make_index_sequence<count>{});
}


template<typename T, typename Function, size_t... Is>
    requires CanReflect<T>
void ForEachIndexedImpl(
    T &&t,
    Function &&function,
    std::index_sequence<Is...>)
{
    auto &&members = GetMemberTuple(ForwardReflector<T>(std::forward<T>(t)));
    using Members = decltype(members);

    if constexpr (HasAliases<T>)
    {
        (function.template operator()<Is>(
            GetFieldNames<T, Is>(),
            ForwardGet<Is>(std::forward<Members>(members))), ...);
    }
    else
    {
        static constexpr auto names = MemberNames<ReflectorType<T>>;

        (function.template operator()<Is>(
            FieldNames<>(std::get<Is>(names)),
            ForwardGet<Is>(std::forward<Members>(members))), ...);
    }
}


template<typename T, typename Function>
    requires CanReflect<T>
void ForEachIndexed(T &&t, Function &&function)
{
    static constexpr auto count = GetMemberCount<ReflectorType<T>>();

    ForEachIndexedImpl(
        ForwardReflector<T>(std::forward<T>(t)),
        std::forward<Function>(function),
        std::make_index_sequence<count>{});
}



template<typename Left, typename Right, typename Function, size_t... Is>
    requires (CanReflect<Left> && CanReflect<Right>)
void ForEachZipImpl(
    Left &&left,
    Right &&right,
    Function &&function, std::index_sequence<Is...>)
{
    auto &&leftMembers =
        GetMemberTuple(ForwardReflector<Left>(std::forward<Left>(left)));

    auto &&rightMembers =
        GetMemberTuple(ForwardReflector<Right>(std::forward<Right>(right)));

    using LeftMembers = decltype(leftMembers);
    using RightMembers = decltype(rightMembers);

    static constexpr auto names = MemberNames<Left>;

    constexpr bool acceptsNames = (
        std::is_invocable_v
        <
            Function &,
            decltype(std::get<Is>(names)),

            decltype(ForwardGet<Is>(
                std::forward<LeftMembers>(leftMembers))),

            decltype(ForwardGet<Is>(
                std::forward<RightMembers>(rightMembers)))
        > && ...
    );

    if constexpr (acceptsNames)
    {
        (function(
            std::get<Is>(names),
            ForwardGet<Is>(std::forward<LeftMembers>(leftMembers)),
            ForwardGet<Is>(std::forward<RightMembers>(rightMembers))), ...);
    }
    else
    {
        // Function only expects the left and right members.

        (function(
            ForwardGet<Is>(std::forward<LeftMembers>(leftMembers)),
            ForwardGet<Is>(std::forward<RightMembers>(rightMembers))), ...);
    }
}


template<typename Left, typename Right, typename Function>
    requires (CanReflect<Left> && CanReflect<Right>)
void ForEachZip(Left &&left, Right &&right, Function &&function)
{
    static constexpr auto count = GetMemberCount<ReflectorType<Left>>();

    static_assert(
        GetMemberCount<ReflectorType<Right>>() == count,
        "Both left and right must have the same member count");

    ForEachZipImpl(
        ForwardReflector<Left>(std::forward<Left>(left)),
        ForwardReflector<Right>(std::forward<Right>(right)),
        std::forward<Function>(function),
        std::make_index_sequence<count>{});
}


} // end namespace fields
