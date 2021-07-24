/**
  * @file marshal.h
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
#include "jive/numeric_string_compare.h"
#include "fields/detail/marshal_detail.h"
#include "fields/describe.h" // MakeIndent

namespace fields
{


struct DefaultBooleans
{
    static constexpr auto trueString = "true";
    static constexpr auto falseString = "false";
};


template<unsigned width>
std::string MakeIndentedLine(int indent)
{
    if (indent < 0)
    {
        return " ";
    }

    return "\n" + std::string(static_cast<unsigned>(indent) * width, ' ');
}


template<typename Booleans>
class MarshalTemplate
{
public:
    using This = MarshalTemplate<Booleans>;

    using Map = std::map<
        std::string,
        std::unique_ptr<This>,
        jive::NumericStringCompare>;

    using const_iterator = typename Map::const_iterator;

    MarshalTemplate() = default;

    template<
        typename T,
        typename = typename std::enable_if<(
            std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)>>
    MarshalTemplate(T value)
        :
        value_(jive::PreciseString(value))
    {

    }

    MarshalTemplate(const std::string &value)
        :
        value_(value)
    {

    }

    MarshalTemplate(bool value)
        :
        value_((value) ? Booleans::trueString : Booleans::falseString)
    {

    }

    template<
        typename T,
        typename = typename std::enable_if_t<(
            std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)>>
    operator T () const { return detail::ToNumber<T>(this->value_); }

    operator bool () const
    {
        return (this->value_ == Booleans::trueString);
    }

    operator const std::string & () const { return this->value_; }

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
        this->value_ = (value) ? Booleans::trueString : Booleans::falseString;
    }

    This & operator=(const std::string &value)
    {
        this->value_ = value;
        return *this;
    }

    size_t count(const std::string &name) const
    {
        return this->membersByName_.count(name);
    }

    template<typename Key>
    This & operator[](const Key &name)
    {
        auto it = this->membersByName_.find(name);

        if (it == this->membersByName_.end())
        {
            // This member does not exist yet.
            // Create it.
            std::tie(it, std::ignore) = this->membersByName_.emplace(
                name,
                std::make_unique<This>());
        }

        return *it->second;
    }

    template<typename Key>
    const This & operator[](const Key &name) const
    {
        auto it = this->membersByName_.find(name);
        if (it == this->membersByName_.end())
        {
            throw std::out_of_range("Member not found");
        }

        return *it->second;
    }

    This & at(const std::string &name)
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

    const_iterator begin() const { return this->membersByName_.begin(); }
    const_iterator end() const { return this->membersByName_.end(); }

    template<typename Parameters>
    std::ostream & Serialize(std::ostream &outputStream, int indent = -1)
    {
        outputStream << this->value_;

        if (!this->membersByName_.empty())
        {
            for (const auto & [name, member]: this->membersByName_)
            {
                outputStream
                    << MakeIndentedLine<Parameters::indentWidth>(indent)
                    << name;

                if (!member->value_.empty())
                {
                    outputStream << Parameters::separator;
                }

                member->template Serialize<Parameters>(
                    outputStream,
                    indent + 1);
            }
        }

        return outputStream;
    }

private:
    std::string value_;
    Map membersByName_;
};


using Marshal = MarshalTemplate<DefaultBooleans>;


} // end namespace fields
