#pragma once


#include <jive/for_each.h>
#include <fields/has_fields.h>


namespace fields
{


// Call a function for each field
template <typename T, typename F>
constexpr void ForEachField(F &&function)
{
    static_assert(HasFields<T>, "Missing required fields tuple");
    jive::ForEach(T::fields, std::forward<F>(function));
}


// Call a function for each field, passing the index instead of the field
// directly.
template <typename T, typename F>
constexpr void ForEachFieldIndexed(F &&function)
{
    static_assert(HasFields<T>, "Missing required fields tuple");

    using Object = std::remove_cvref_t<T>;
    constexpr auto count = std::tuple_size_v<decltype(Object::fields)>;

    [&]<size_t... Is>(std::index_sequence<Is...>)
    {
        (static_cast<void>(
            function.template operator()<Is>()), ...);
    }(std::make_index_sequence<count>{});
}


}
