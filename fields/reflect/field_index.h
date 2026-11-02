#pragma once

#include <fields/reflect/reflect.h>
#include <fields/reflect/fixed_string.h>


namespace fields
{


template<FixedString TargetName, typename Struct, size_t... Is>
    requires HasFields<Struct>
consteval size_t FieldIndexImpl(std::index_sequence<Is...>)
{
    auto fieldsTuple = Struct::fields;
    size_t result = sizeof...(Is);
    ((std::get<Is>(fieldsTuple).name == TargetName ? (result = Is) : 0), ...);

    return result;
}

template<FixedString TargetName, typename Struct, size_t... Is>
    requires CanReflect<Struct>
consteval size_t FieldIndexImpl(std::index_sequence<Is...>)
{
    size_t result = sizeof...(Is);

    (
        (Reflect<Struct>::template name<Is> == TargetName
            ? (result = Is)
            : 0),
        ...);

    return result;
}


template<FixedString Name, typename Struct>
consteval size_t FieldIndex()
{
    constexpr size_t fieldCount = ReflectMemberCount<Struct>();

    return FieldIndexImpl<Name, Struct>(
        std::make_index_sequence<fieldCount>{});

    constexpr size_t index = FieldIndexImpl<Name, Struct>(
            std::make_index_sequence<fieldCount>{});

    static_assert(
        index < fieldCount,
        "Field name not found in Struct");

    return index;
}


} // end namespace fields
