#pragma once


#include <jive/type_traits.h>
#include <fields/field.h>
#include <fields/reflect/get_member_tuple.h>
#include <fields/reflect/reflect.h>
#include <fields/core_member_type.h>


namespace fields
{


template<typename T>
concept FieldsHidden = std::remove_reference_t<T>::fieldsHidden;


template<typename T>
concept HasHidden = requires
{
    std::remove_cvref_t<T>::hidden;
    requires IsHidesTuple<decltype(std::remove_cvref_t<T>::hidden)>;
};


template
<
    typename T,
    size_t index,
    IsAddressHide HideType
>
    requires (MustReflect<T>)
consteval bool GetIsMemberHidden(const HideType &hidden)
{
    using Reflector = ReflectorType<T>;

    static_assert(
        jive::ConstevalDefaultConstructible<Reflector>,
        "ReflectorType<T> must be constexpr default constructible. "
        "One or more reflected members cannot be constructed at compile time. "
        "Use IndexHide instead");

    Reflector object{};
    const auto &member = std::get<index>(GetMemberTuple(object));
    const auto memberPointer = &member;
    using Member = std::remove_cvref_t<decltype(member)>;

    if constexpr (std::same_as<Member, typename HideType::Type>)
    {
        return memberPointer == &(object.*hidden.member);
    }
    else
    {
        return false;
    }
}


template
<
    typename T,
    size_t index,
    IsIndexHide HideType
>
consteval bool GetIsMemberHidden(const HideType &)
{
    return index == HideType::index;
}


template
<
    typename T,
    size_t index,
    IsAddressField Field,
    IsAddressHide HideType
>
consteval bool GetIsMemberFieldHidden(
    const Field &field,
    const HideType &hidden)
{
    if constexpr (std::same_as<typename Field::Type, typename HideType::Type>)
    {
        return (field.member == hidden.member);
    }
    else
    {
        return false;
    }
}


template
<
    CanReflect T,
    size_t index,
    IsIndexField Field,
    IsAddressHide HideType
>
consteval bool GetIsMemberFieldHidden(
    const Field &,
    const HideType &hidden)
{
    using Reflector = ReflectorType<T>;

    static_assert(
        jive::ConstevalDefaultConstructible<Reflector>,
        "ReflectorType<T> must be constexpr default constructible. "
        "One or more reflected members cannot be constructed at compile time.");

    Reflector object{};
    const auto &member = std::get<index>(GetMemberTuple(object));
    const auto memberPointer = &member;

    if constexpr (
        std::same_as<CoreMemberType<T, Field>, typename HideType::Type>)
    {
        return (memberPointer == &(object.*hidden.member));
    }
    else
    {
        return false;
    }
}


template
<
    typename T,
    size_t index,
    IsAddressHide HideType
>
    requires (HasFields<T>)
consteval bool GetIsMemberHidden(const HideType &hidden)
{
    constexpr auto memberField =
        std::get<index>(std::remove_cvref_t<T>::fields);

    return GetIsMemberFieldHidden<T, index>(memberField, hidden);
}


template
<
    HasHidden T,
    size_t memberIndex,
    size_t hiddenIndex = 0
>
struct IsHidden;

namespace detail
{

    template
    <
        bool isMemberField,
        HasHidden T,
        size_t memberIndex,
        size_t hiddenIndex
    >
    struct DoFindHidden;

    template
    <
        HasHidden T,
        size_t memberIndex,
        size_t hiddenIndex
    >
    struct DoFindHidden
    <
        true,
        T,
        memberIndex,
        hiddenIndex
    >: std::true_type
    {

    };

    template
    <
        HasHidden T,
        size_t memberIndex,
        size_t hiddenIndex
    >
    struct DoFindHidden
    <
        false,
        T,
        memberIndex,
        hiddenIndex
    > : IsHidden<T, memberIndex, hiddenIndex + 1>
    {

    };

} // end namespace detail


template
<
    HasHidden T,
    size_t memberIndex,
    size_t hiddenIndex
>
struct IsHidden
    :
    detail::DoFindHidden
    <
        GetIsMemberHidden<T, memberIndex>(
            std::get<hiddenIndex>(std::remove_cvref_t<T>::hidden)),
        T,
        memberIndex,
        hiddenIndex
    >
{

};


template
<
    HasHidden T,
    size_t memberIndex,
    size_t hiddenIndex
>
// Stop condition
// We searched all of the hidden for this member, and did not find it.
    requires (
        hiddenIndex
            == std::tuple_size_v<decltype(std::remove_cvref_t<T>::hidden)>)
struct IsHidden
<
    T,
    memberIndex,
    hiddenIndex
>: std::false_type
{

};


template<CanInspect T, typename = void>
struct ExcludeHidden
{
};


template<CanInspect T>
struct ExcludeHidden
<
    T,
    std::enable_if_t<HasHidden<T>>
>
{
    template<typename Member, size_t memberIndex>
    struct Exclude
    {
        static constexpr bool value =
            FieldsHidden<Member> || IsHidden<T, memberIndex>::value;
    };
};


template<CanInspect T>
struct ExcludeHidden
<
    T,
    std::enable_if_t<!HasHidden<T>>
>
{
    template<typename Member, size_t>
    struct Exclude
    {
        static constexpr bool value = FieldsHidden<Member>;
    };
};


template<CanInspect T, typename Member, size_t index>
constexpr bool GetExcludeHidden()
{
    return ExcludeHidden<T>::template Exclude<Member, index>::value;
}


template<typename T, IsAddressField FieldType, IsAddressHide HideType>
constexpr bool IsHiddenField(const FieldType &field, const HideType &hide)
{
    if constexpr (
        std::same_as<typename FieldType::Type, typename HideType::Type>)
    {
        return field.member == hide.member;
    }
    else
    {
        return false;
    }
}


template<typename T, IsIndexField FieldType, IsIndexHide HideType>
constexpr bool IsHiddenField(const FieldType &field, const HideType &hide)
{
    return field.index == hide.index;
}


// Mixed index/address style requires us to get the address of the member,
// which requires consteval construction of T.
template<typename T, IsIndexField FieldType, IsAddressHide HideType>
    requires(jive::ConstevalDefaultConstructible<T>)
constexpr bool IsHiddenField(const FieldType &, const HideType &hide)
{
    using Reflector = ReflectorType<T>;

    Reflector object{};
    const auto &member = std::get<FieldType::index>(GetMemberTuple(object));
    const auto memberPointer = &member;
    using Member = std::remove_cvref_t<decltype(member)>;

    if constexpr (std::same_as<Member, typename HideType::Type>)
    {
        return memberPointer == &(object.*hide.member);
    }
    else
    {
        return false;
    }
}

template<typename T, IsAddressField FieldType, IsIndexHide HideType>
    requires(jive::ConstevalDefaultConstructible<T>)
constexpr bool IsHiddenField(const FieldType &field, const HideType &)
{
    using Reflector = ReflectorType<T>;

    Reflector object{};
    const auto &member = std::get<HideType::index>(GetMemberTuple(object));
    const auto memberPointer = &member;
    using Member = std::remove_cvref_t<decltype(member)>;

    if constexpr (std::same_as<Member, typename FieldType::Type>)
    {
        return memberPointer == &(object.*field.member);
    }
    else
    {
        return false;
    }
}


template<typename T, IsAddressField FieldType>
constexpr bool GetIsHiddenField(const FieldType &field)
{
    using Type = std::remove_cvref_t<T>;

    bool result = false;

    using Member = typename FieldType::Type;

    if constexpr (FieldsHidden<Member>)
    {
        return true;
    }
    else if constexpr (HasHidden<Type>)
    {
        using HiddenType = decltype(Type::hidden);

        [&]<std::size_t...Is>(std::index_sequence<Is...>)
        {
            result =
                (IsHiddenField<Type>(field, std::get<Is>(Type::hidden)) || ...);
        }(std::make_index_sequence<std::tuple_size_v<HiddenType>>{});

        return result;
    }
    else
    {
        return false;
    }
}


template<typename T, IsIndexField FieldType>
constexpr bool GetIsHiddenField(const FieldType &field)
{
    using Type = std::remove_cvref_t<T>;

    bool result = false;

    if constexpr (HasHidden<Type>)
    {
        using HiddenType = decltype(Type::hidden);

        [&]<std::size_t...Is>(std::index_sequence<Is...>)
        {
            result =
                (IsHiddenField<Type>(field, std::get<Is>(Type::hidden)) || ...);
        }(std::make_index_sequence<std::tuple_size_v<HiddenType>>{});

        return result;
    }
    else
    {
        return false;
    }
}


} // end namespace fields
