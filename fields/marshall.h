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

#include <memory>
#include <map>
#include <vector>
#include "jive/precise_string.h"
#include "fields/detail/marshall_detail.h"

namespace fields
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

    size_t count(const std::string &name) const
    {
        return this->membersByName_.count(name);
    }

    template<typename Key>
    Marshall & operator[](const Key &name)
    {
        auto it = this->membersByName_.find(name);

        if (it == this->membersByName_.end())
        {
            // This member does not exist yet.
            // Create it.
            std::tie(it, std::ignore) = this->membersByName_.emplace(
                name,
                std::make_unique<Marshall>());
        }

        return *it->second;
    }

    template<typename Key>
    const Marshall & operator[](const Key &name) const
    {
        auto it = this->membersByName_.find(name);
        if (it == this->membersByName_.end())
        {
            throw std::out_of_range("Member not found");
        }

        return *it->second;
    }
    /**

    template<size_t N>
    Marshall & operator[](const char (&name)[N])
    {
        return this->operator[](std::string(name));
    }

    template<size_t N>
    const Marshall & operator[](const char (&name)[N]) const
    {
        return this->operator[](std::string(name));
    }
**/
    Marshall & at(const std::string &name)
    {
        return *(this->membersByName_.at(name));
    }

    std::vector<std::string> GetNames() const
    {
        std::vector<std::string> result;
        result.reserve(this->membersByName_.size());

        for (auto &it: this->membersByName_)
        {
            result.push_back(it.first);
        }

        return result;
    }

    size_t size() const { return this->membersByName_.size(); }

private:
    std::string value_;

    std::map<std::string, std::unique_ptr<Marshall>, std::less<>>
        membersByName_;
};

} // end namespace fields
