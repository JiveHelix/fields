#pragma once


#include <jive/describe_type.h>
#include <jive/optional.h>

#include <fields/has_fields.h>
#include <fields/reflect/get_member_count.h>
#include <fields/reflect/member_names.h>


namespace fields
{


template<typename T>
concept DefinesReflector = requires
{
    typename std::remove_cvref_t<T>::Reflector;
};


template<typename T, typename = void>
struct GetReflectorImpl {};


template<typename T>
struct GetReflectorImpl
<
    T,
    std::enable_if_t<DefinesReflector<T>>
>
{
    using Type = std::remove_cvref_t<T>::Reflector;
};


template<typename T>
using GetReflector = typename GetReflectorImpl<T>::Type;


template<typename T>
constexpr bool CheckMemberCountIsOkay()
{
    if constexpr (!HasFields<T>)
    {
        // We must use reflection to inspect the members.
        // Check the member count.

        constexpr auto memberCount = GetMemberCount<T>();

        return (memberCount > 0) && (memberCount <= maximumReflectCount);
    }
    else
    {
        // fields can specify as many members as necessary.
        return true;
    }
}



template<typename T>
concept CanReflectImpl =
    std::is_aggregate_v<T>
    && !fields::HasFields<T>
    && !std::is_array_v<T>
    && !jive::IsArray<T>
    && !std::is_pointer_v<T>
    && !jive::IsOptional<T>
    && !jive::IsValueContainer<T>
    && CheckMemberCountIsOkay<T>();


template<typename T, typename = void>
struct DefaultReflectorTypeImpl
{

};


template<typename T>
struct DefaultReflectorTypeImpl
<
    T,
    std::enable_if_t
    <
        !DefinesReflector<T>
        && CanReflectImpl<T>
    >
>
{
    using Type = T;
};


template<typename T>
struct DefaultReflectorTypeImpl
<
    T,
    std::enable_if_t
    <
        DefinesReflector<T>
        && CanReflectImpl<GetReflector<T>>
    >
>
{
    using Type = GetReflector<T>;
};


// Define the default through inheritance.
// Downstream libraries can specialize ReflectorTypeImpl without competeting
// with the default behavior.
template<typename T, typename = void>
struct ReflectorTypeImpl: DefaultReflectorTypeImpl<T>
{

};


template<typename T>
using ReflectorType = typename ReflectorTypeImpl<std::remove_cvref_t<T>>::Type;


template<typename T>
concept HasReflector = requires
{
    typename ReflectorType<T>;
    requires !HasFields<T>;
};


template<typename T>
concept CanReflect =
    CanReflectImpl<std::remove_cvref_t<T>> || HasReflector<T>;


template<typename T>
    requires CanReflect<T>
constexpr auto && ForwardReflector(T &&t)
{
    using Type = ReflectorType<T>;

    using Base = std::conditional_t
        <
            std::is_const_v<std::remove_reference_t<T>>,
            const Type,
            Type
        >;

    using Reference = std::conditional_t
        <
            std::is_lvalue_reference_v<T>,
            Base &,
            Base &&
        >;

    return static_cast<Reference>(t);
}


template<CanReflect T>
struct Reflect
{
    using Type = ReflectorType<T>;

    static constexpr auto count = GetMemberCount<Type>();

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
    using Members = decltype(GetMemberTuple(inspect<Type>));
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    static constexpr auto names = MemberNames<Type>;

    template<size_t I>
    using Element = std::remove_cvref_t<std::tuple_element_t<I, Members>>;

    template<size_t I>
    static constexpr auto name = std::get<I>(names);
};



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
    requires CanReflect<T>
void ForEach(T &&t, Function &&function)
{
    static constexpr auto count = GetMemberCount<ReflectorType<T>>();

    ForEachImpl(
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


template<CanReflect T>
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
