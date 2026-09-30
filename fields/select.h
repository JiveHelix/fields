#pragma once


#include <fields/reflect/reflect_traits.h>


namespace fields
{


namespace detail
{


/**
    Build up a list of indices to fields that meet our criterion.
    Called initially with only three template arguments.
    On the first pass, the last field is checked for inclusion, and added to
    the pack of size_t as the last template argument if it is not empty.
    Otherwise, this method is called again with the penultimate field, and
    so on.
    When Count reaches 0, we have considered all of the fields, and can
    return the final index_sequence.
**/
template
<
    template<typename, size_t> typename Exclude,
    typename T,
    typename Fields,
    size_t Count,
    size_t... I
>
constexpr auto SelectFields(const T &object, const Fields &fields)
{
    if constexpr (Count == 0)
    {
        return std::index_sequence<I...>();
    }
    else
    {
        using MemberType = typename std::remove_reference_t
            <
                decltype(object.*(std::get<Count - 1>(fields).member))
            >;

        if constexpr (Exclude<MemberType, Count - 1>::value)
        {
            // Exclude this field.
            return SelectFields<Exclude, T, Fields, Count - 1, I...>(
                object,
                fields);
        }
        else
        {
            return SelectFields<Exclude, T, Fields, Count - 1, Count - 1, I...>(
                object,
                fields);
        }
    }
}


template
<
    template<typename, size_t> typename Exclude,
    typename T,
    typename Reflection,
    size_t Count,
    size_t... Is
>
constexpr auto SelectMembers(const T &object)
{
    if constexpr (Count == 0)
    {
        return std::index_sequence<Is...>();
    }
    else
    {
        using MemberType = typename Reflection::template Element<Count - 1>;

        if constexpr (Exclude<MemberType, Count - 1>::value)
        {
            // Empty types do not participate in comparisons.
            // Skip this field.
            return SelectMembers
                <
                    Exclude,
                    T,
                    Reflection,
                    Count - 1,
                    Is...
                >(object);
        }
        else
        {
            // Add this index to the members that will be selected.
            return SelectMembers
                <
                    Exclude,
                    T,
                    Reflection,
                    Count - 1,
                    Count - 1,
                    Is...
                >(object);
        }
    }
}


} // end namespace detail


template
<
    template<typename, size_t> typename Exclude,
    CanInspect T
>
constexpr auto SelectIndices(const T &object)
{
    if constexpr (fields::HasFields<T>)
    {
        using Fields = decltype(T::fields);
        constexpr auto propertyCount = std::tuple_size_v<Fields>;

        return detail::SelectFields<Exclude, T, Fields, propertyCount>(
            object,
            T::fields);
    }
    else
    {
        static_assert(CanReflect<T>);

        using Reflection = Reflect<T>;

        return detail::SelectMembers
            <
                Exclude,
                T,
                Reflection,
                Reflection::count
            >(object);
    }
}


} // end namespace fields
