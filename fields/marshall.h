/**
  * @file marshall.h
  *
  * @brief Convert arithmetic types to and from std::string.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 09 Jul 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include "jive/precise_string.h"
#include "fields/detail/marshall_detail.h"

namespace jive
{

class Marshall
{
public:
    Marshall() = default;

    template<
        typename T,
        typename = typename std::enable_if<(
            std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)>>
    Marshall(T value)
        :
        value_(jive::PreciseString(value))
    {

    }

    Marshall(const std::string &value)
        :
        value_(value)
    {

    }

    Marshall(bool value)
        :
        value_((value) ? "true" : "false")
    {

    }

    template<
        typename T,
        typename = typename std::enable_if_t<(
            std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)>>
    operator T () const { return detail::ToNumber<T>(this->value_); }

    operator bool () const
    {
        return (this->value_ == "true");
    }

    operator std::string () const { return this->value_; }

    template<
        typename T,
        typename = typename std::enable_if<(
            std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)>>
    void operator=(T value)
    {
        this->value_ = jive::PreciseString(value);
    }

    void operator=(bool value)
    {
        this->value_ = (value) ? "true" : "false";
    }

    void operator=(const std::string &value)
    {
        this->value_ = value;
    }

private:
    std::string value_;
};

} // end namespace jive
