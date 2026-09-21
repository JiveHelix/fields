#pragma once


#include <jive/describe_type.h>
#include <jive/optional.h>

#include <fields/has_fields.h>
#include <fields/reflect/get_member_count.h>
#include <fields/reflect/member_names.h>


namespace fields
{


template<typename T>
concept CanReflect =
    std::is_aggregate_v<std::remove_cvref_t<T>>
    && !fields::HasFields<T>
    && !std::is_array_v<T>
    && !jive::IsArray<T>
    && !std::is_pointer_v<T>
    && !jive::IsOptional<T>
    && !jive::IsValueContainer<T>
    && (GetMemberCount<T>() > 0)
    && (GetMemberCount<T>() < maximumReflectCount);


template<CanReflect T>
struct Reflect
{
    static constexpr auto count = GetMemberCount<T>();

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
    using Members = decltype(GetMemberTuple(inspect<T>));
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    static constexpr auto names = MemberNames<T>;

    template<size_t I>
    using Element = std::remove_cvref_t<std::tuple_element_t<I, Members>>;

    template<size_t I>
    static constexpr auto name = std::get<I>(names);
};



template<size_t Index, typename T>
    requires CanReflect<std::remove_cvref_t<T>>
decltype(auto) GetMember(T &&t)
{
    return std::get<Index>(GetMemberTuple(std::forward<T>(t)));
}


template<size_t I, typename Tuple>
decltype(auto) ForwardGet(Tuple &&tuple)
{
    using T = decltype(std::get<I>(tuple));
    return std::forward<T>(std::get<I>(tuple));
}


template<typename T, typename Function, size_t... Is>
    requires CanReflect<std::remove_cvref_t<T>>
void ForEachImpl(T &&t, Function &&function, std::index_sequence<Is...>)
{
    auto &&members = GetMemberTuple(t);
    using Members = decltype(members);

    static constexpr auto names = MemberNames<T>;

    constexpr bool acceptsNames = (
        std::is_invocable_v
        <
            Function &,
            decltype(std::get<Is>(names)),

            decltype(ForwardGet<Is>(
                std::forward<Members>(members)))
        > && ...
    );

    if constexpr (acceptsNames)
    {
        (function(
            std::get<Is>(names),
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
    requires CanReflect<std::remove_cvref_t<T>>
void ForEach(T &&t, Function &&function)
{
    static constexpr auto count = GetMemberCount<T>();

    ForEachImpl(
        std::forward<T>(t),
        std::forward<Function>(function),
        std::make_index_sequence<count>{});
}


template<typename Left, typename Right, typename Function, size_t... Is>
    requires (
        CanReflect<std::remove_cvref_t<Left>>
        && CanReflect<std::remove_cvref_t<Right>>)
void ForEachZipImpl(
    Left &&left,
    Right &&right,
    Function &&function, std::index_sequence<Is...>)
{
    auto &&leftMembers = GetMemberTuple(left);
    auto &&rightMembers = GetMemberTuple(right);
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
    requires (
        CanReflect<std::remove_cvref_t<Left>>
        && CanReflect<std::remove_cvref_t<Right>>)
void ForEachZip(Left &&left, Right &&right, Function &&function)
{
    static constexpr auto count = GetMemberCount<Left>();
    static_assert(
        GetMemberCount<Right>() == count,
        "Both left and right must have the same member count");

    ForEachZipImpl(
        std::forward<Left>(left),
        std::forward<Right>(right),
        std::forward<Function>(function),
        std::make_index_sequence<count>{});
}


template<typename T>
void PrintMemberTypes(std::ostream &output)
{
    using Reflection = Reflect<T>;

    [&output]<size_t... Is>(std::index_sequence<Is...>)
    {
        ((output << "name: " << std::get<Is>(Reflection::names)
            << ", type: "
            << jive::GetTypeName<typename Reflection::template Element<Is>>()
            << '\n'), ...);
    }(std::make_index_sequence<Reflection::count>{});
}


} // end namespace fields
