/**
  * @file compare.h
  * 
  * @brief Implements comparison operator overloads for fields classes.
  * 
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 01 May 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */
#pragma once

#include "fields/core.h"

namespace fields
{

template<typename T>
std::enable_if_t<HasFields<T>::value, bool>
operator==(const T &left, const T &right)
{
    return GetFields(left) == GetFields(right);
}


template<typename T>
std::enable_if_t<HasFields<T>::value, bool>
operator!=(const T &left, const T &right)
{
    return GetFields(left) != GetFields(right);
}


template<typename T>
std::enable_if_t<HasFields<T>::value, bool>
operator<(const T &left, const T &right)
{
    return GetFields(left) < GetFields(right);
}


template<typename T>
std::enable_if_t<HasFields<T>::value, bool>
operator>(const T &left, const T &right)
{
    return GetFields(left) > GetFields(right);
}


template<typename T>
std::enable_if_t<HasFields<T>::value, bool>
operator<=(const T &left, const T &right)
{
    return !(left > right);
}


template<typename T>
std::enable_if_t<HasFields<T>::value, bool>
operator>=(const T &left, const T &right)
{
    return !(left < right);
}

} // end namespace fields
