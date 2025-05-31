#pragma once


#include <jive/for_each.h>


namespace fields
{


// Call a function for each field
template <typename T, typename F>
constexpr void ForEachField(F &&function)
{
    static_assert(HasFields<T>, "Missing required fields tuple");
    jive::ForEach(T::fields, std::forward<F>(function));
}





}
