#pragma once


#include <jive/type_traits.h>
#include <fields/field.h>
#include <fields/reflect/get_member_tuple.h>
#include <fields/reflect/reflect.h>


namespace fields
{


template<typename T>
concept HasAliases = requires
{
    std::remove_cvref_t<T>::aliases;
    requires IsFieldsTuple<decltype(std::remove_cvref_t<T>::aliases)>;
};


template
<
    CanReflect T,
    size_t index,
    IsField FieldType
>
consteval bool GetIsMemberField(const FieldType &field)
{
    using Reflector = ReflectorType<T>;

    static_assert(
        jive::ConstevalDefaultConstructible<Reflector>,
        "ReflectorType<T> must be constexpr default constructible. "
        "One or more reflected members cannot be constructed at compile time.");

    Reflector object{};
    const auto &member = std::get<index>(GetMemberTuple(object));
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


template
<
    CanReflect T,
    IsField FieldType,
    typename Member
>
constexpr bool GetIsMemberField(
    const FieldType &field,
    T &&object,
    Member &&member)
{
    return &member == &(object.*field.member);
}


template
<
    HasAliases T,
    size_t memberIndex,
    size_t fieldIndex = 0
>
struct FindFieldNames;

namespace detail
{

    template
    <
        bool isMemberField,
        HasAliases T,
        size_t memberIndex,
        size_t fieldIndex
    >
    struct DoFindFieldNames;

    template
    <
        HasAliases T,
        size_t memberIndex,
        size_t fieldIndex
    >
    struct DoFindFieldNames
    <
        true,
        T,
        memberIndex,
        fieldIndex
    >
    {
        static constexpr auto value =
            CreateFieldNames(
                Reflect<T>::template name<memberIndex>,
                std::get<fieldIndex>(std::remove_cvref_t<T>::aliases));
    };

    template
    <
        HasAliases T,
        size_t memberIndex,
        size_t fieldIndex
    >
    struct DoFindFieldNames
    <
        false,
        T,
        memberIndex,
        fieldIndex
    > : FindFieldNames<T, memberIndex, fieldIndex + 1>
    {

    };

} // end namespace detail


template
<
    HasAliases T,
    size_t memberIndex,
    size_t fieldIndex
>
struct FindFieldNames
    :
    detail::DoFindFieldNames
    <
        GetIsMemberField<T, memberIndex>(
            std::get<fieldIndex>(std::remove_cvref_t<T>::aliases)),
        T,
        memberIndex,
        fieldIndex
    >
{

};


template
<
    HasAliases T,
    size_t memberIndex,
    size_t fieldIndex
>
// Stop condition
    requires (
        fieldIndex
            == std::tuple_size_v<decltype(std::remove_cvref_t<T>::aliases)>)
struct FindFieldNames
<
    T,
    memberIndex,
    fieldIndex
>
{
    // We searched all of the Fields for this member, and did not find it.
    // Create FieldNames using reflection.
    static constexpr auto value =
        FieldNames(Reflect<T>::template name<memberIndex>);
};


template
<
    HasAliases T,
    size_t memberIndex
>
constexpr auto GetFieldNames()
{
    return FindFieldNames<T, memberIndex>::value;
}



} // end namespace fields
