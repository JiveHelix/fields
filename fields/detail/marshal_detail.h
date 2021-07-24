/**
  * @file marshal_detail.h
  *
  * @brief A helper method for unmarshaling values.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 09 Jul 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <type_traits>
#include "jive/to_integer.h"
#include "jive/to_float.h"

namespace fields
{

namespace detail
{

template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>, T>
ToNumber(const std::string &value)
{
    if constexpr (std::is_integral_v<T>)
    {
        return jive::ToInteger<T>(value);
    }
    else
    {
        static_assert(std::is_floating_point_v<T>);
        return jive::ToFloat<T>(value);
    }
}

} // end namespace detail

} // end namespace fields
