#pragma once

#include <type_traits>
#include <jive/type_traits.h>
#include <jive/optional.h>
#include <fields/has_fields.h>
#include <fields/reflect/get_member_count.h>


namespace fields
{


template<typename T>
concept DefinesReflector = requires
{
    typename std::remove_cvref_t<T>::Reflector;
};


template<typename T>
concept DefinesMembers = requires
{
    typename std::remove_cvref_t<T>::Members;
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
struct GetReflectorImpl
<
    T,
    std::enable_if_t<DefinesMembers<T>>
>
{
    using Type = std::remove_cvref_t<T>::Members;
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
        && !DefinesMembers<T>
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
        (DefinesReflector<T> || DefinesMembers<T>)
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
concept CanInspect = HasFields<T> || CanReflect<T>;


} // end namespace fields
