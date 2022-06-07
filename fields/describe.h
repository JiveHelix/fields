/**
  * @file describe.h
  *
  * @brief Creates a string representation of any fields class.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 01 May 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#pragma once

#include <ostream>
#include <sstream>
#include <cassert>
#include "fields/core.h"
#include "jive/describe_type.h"
#include "jive/colorize.h"
#include "jive/type_traits.h"


namespace jive
{

// Define DescribeType for fields classes
template<typename T>
struct DescribeType<T, std::enable_if_t<fields::HasFieldsTypeName<T>::value>>
{
    static constexpr std::string_view value = T::fieldsTypeName;
};

} // end namespace jive

namespace fields
{

/** Optionally, a class can provide it's own Describe method.
 ** Must be a templated member method like
 **
 ** template<typename Colors, typename VerboseTypes>
 ** std::ostream & Describe(std::ostream &, int indent);
 **
 **/
template<typename T, typename Colors, typename VerboseTypes, typename = void>
struct ImplementsDescribe: std::false_type {};

template<typename T, typename Colors, typename VerboseTypes>
struct ImplementsDescribe<
    T,
    Colors,
    VerboseTypes,
    std::void_t<
        std::enable_if_t<
            std::is_same_v<
                std::ostream &,
                decltype(
                    std::declval<T>().template Describe<
                        Colors,
                        VerboseTypes>(std::declval<std::ostream &>(), int()))
            >
        >
    >
>: std::true_type {};


struct DefaultColors
{
    static constexpr auto name = jive::color::green;
    static constexpr auto structure = jive::color::cyan;
    static constexpr auto type = jive::color::yellow;
};

struct NoColor
{
    static constexpr auto name = "";
    static constexpr auto structure = "";
    static constexpr auto type = "";
};


template<
    typename T,
    typename Colors = NoColor,
    typename VerboseTypes = std::false_type>
class Describe;


template<typename T, typename Colors = DefaultColors>
auto DescribeColorized(const T &object, int indent = -1)
{
    return Describe<T, Colors>(object, indent);
}


template<typename T, typename Colors = DefaultColors>
auto DescribeColorizedVerbose(const T &object, int indent = -1)
{
    return Describe<T, Colors, std::true_type>(object, indent);
}


template<typename T, typename Colors = DefaultColors>
auto DescribeCompact(const T &object, int indent = -1)
{
    return Describe<T, Colors, std::false_type>(object, indent);
}


template<typename T, typename Colors, typename VerboseTypes>
std::string ToString(const Describe<T, Colors, VerboseTypes> &describe)
{
    std::ostringstream outputStream;
    describe(outputStream);
    return outputStream.str();
}


template<typename T, typename Colors, typename VerboseTypes>
std::ostream & operator<<(
    std::ostream &outputStream,
    const Describe<T, Colors, VerboseTypes> &describe)
{
    return describe(outputStream);
}


inline std::string MakeIndent(int indent)
{
    if (indent <= 0)
    {
        return {};
    }

    return "\n" + std::string(static_cast<unsigned>(indent) * 4, ' ');
}


template<typename T, typename Colors, typename VerboseTypes>
class Describe
{
public:
    Describe(const T &object, const std::string &name, int indent = -1)
        :
        object_{object},
        name_{name},
        indent_{indent}
    {

    }

    Describe(const T &object, int indent = -1)
        :
        object_{object},
        name_{},
        indent_{indent}
    {

    }

    // We are storing a const reference. Disallow temporaries.
    Describe(T &&object, const std::string &name) = delete;

    std::string GetIndent() const
    {
        return MakeIndent(this->indent_);
    }

    template<typename Key>
    static std::string KeyToString(const Key &key)
    {
        return std::to_string(key);
    }

    static std::string KeyToString(const std::string &key)
    {
        return key;
    }

    template<typename Object>
    std::ostream & DescribeMap(
        std::ostream &outputStream,
        const Object &mapLike) const
    {
        outputStream << "{";
        size_t count = 0;

        for (const auto & [key, value]: mapLike)
        {
            outputStream <<
                Describe<typename Object::mapped_type, Colors, VerboseTypes>(
                    value,
                    KeyToString(key),
                    (this->indent_ < 0) ? -1 : this->indent_ + 1);

            ++count;

            if (count < mapLike.size())
            {
                outputStream << ", ";
            }
        }

        return outputStream << "}";
    }

    template<typename Object>
    std::ostream & DescribeContainer(
        std::ostream &outputStream,
        const Object &container) const
    {
        size_t count = 0;
        outputStream << "[";

        for (const auto &value: container)
        {
            outputStream <<
                Describe<typename Object::value_type, Colors, VerboseTypes>(
                    value,
                    std::to_string(count),
                    (this->indent_ < 0) ? -1 : this->indent_ + 1);

            ++count;

            if (count < container.size())
            {
                outputStream << ", ";
            }
        }

        return outputStream << "]";
    }

    template<typename Object, std::size_t N, std::size_t... I>
    std::ostream & DescribeArray(
        std::ostream &outputStream,
        const Object (&array)[N],
        std::index_sequence<I...>) const
    {
        // Use a fold expression to print a comma between each element.
        // Don't print a comma before the first member.
        ((outputStream << (I == 0 ? "[" : ", ") <<
            /* Recursively wrap each member in Describe */
            Describe<Object, Colors, VerboseTypes>(
                array[I],
                (this->indent_ < 0) ? -1 : this->indent_ + 1)
            << (I == N - 1 ? "]" : "")),
         ...);

        return outputStream;
    }

    template<typename Object, std::size_t N>
    std::ostream & DescribeArray(
        std::ostream &outputStream,
        const Object (&array)[N]) const
    {
        return this->DescribeArray(
            outputStream,
            array,
            std::make_index_sequence<N>{});
    }

    template<typename Object, typename Fields, std::size_t... I>
    std::ostream & DescribeFields(
        std::ostream &outputStream,
        const Object &object,
        const Fields &fields,
        std::index_sequence<I...>) const
    {
        // Use a fold expression to print a comma between each member.
        // Don't print a comma before the first member.
        ((outputStream << (I == 0 ? "" : ", ") <<
            /* Recursively wrap each member in Describe */
            Describe<FieldElementType<I, Fields>, Colors, VerboseTypes>(
                object.*(std::get<I>(fields).member),
                std::get<I>(fields).name,
                (this->indent_ < 0) ? -1 : this->indent_ + 1)),
         ...);

        return outputStream;
    }

    template<typename Object, typename Fields>
    std::ostream & DescribeFields(
        std::ostream &outputStream,
        const Object &object,
        const Fields &fields) const
    {
        constexpr auto itemCount = std::tuple_size<Fields>::value;

#if defined _MSC_VER && _MSC_VER <= 1932
        // MSVC is confused by correct C++ syntax...
        return this->template Describe::DescribeFields(
#else
        // Clang and GCC are not
        return this->template DescribeFields(
#endif
            outputStream,
            object,
            fields,
            std::make_index_sequence<itemCount>{});
    }

    std::ostream & operator()(std::ostream &outputStream) const
    {
        jive::Colorize colorize(outputStream);

        outputStream << this->GetIndent();

        if (!this->name_.empty())
        {
            colorize(Colors::name, this->name_, ": ");
        }

        if constexpr (HasFields<T>::value)
        {
            colorize(Colors::structure, jive::GetTypeName<T>());
            outputStream << "(";
        }

        if constexpr (ImplementsDescribe<T, Colors, VerboseTypes>::value)
        {
            this->object_.template Describe<Colors, VerboseTypes>(
                outputStream,
                (this->indent_ < 0) ? -1 : this->indent_ + 1);
            outputStream << ")";
        }
        else if constexpr (HasFields<T>::value)
        {

#if defined _MSC_VER && _MSC_VER <= 1932
        // MSVC is confused by correct C++ syntax...
        this->template Describe::DescribeFields(
#else
        // Clang and GCC are not
        this->template DescribeFields(
#endif
                outputStream,
                this->object_,
                T::fields);

            outputStream << ")";
        }
        else
        {
            // There are no fields classes, so get a string representation of
            // the object directly.
            if constexpr (VerboseTypes::value)
            {
                colorize(Colors::type, jive::GetTypeName<T>());
                outputStream << " = ";
            }

            if constexpr (std::is_same_v<T, bool>)
            {
                outputStream << std::boolalpha << this->object_;
            }
            else if constexpr (std::is_integral_v<T> && sizeof(T) == 1)
            {
                // print the numeric value of single bytes rather than the
                // ASCII character.
                outputStream << int16_t{this->object_};
            }
            else if constexpr (jive::IsKeyValueContainer<T>::value)
            {
                this->DescribeMap(outputStream, this->object_);
            }
            else if constexpr (jive::IsValueContainer<T>::value)
            {
                this->DescribeContainer(outputStream, this->object_);
            }
            else if constexpr (std::is_array_v<T>)
            {
                this->DescribeArray(outputStream, this->object_);
            }
            else if constexpr (std::is_pointer_v<T>)
            {
                if (this->object_ != nullptr)
                {
                    outputStream <<
                        Describe<
                                std::remove_const_t<std::remove_pointer_t<T>>,
                                Colors,
                                VerboseTypes>(
                            *(this->object_),
                            (this->indent_ < 0) ? -1 : this->indent_ + 1);
                }
            }
            else
            {
                outputStream << this->object_;
            }
        }

        return outputStream;
    }

    std::string ToString() const
    {
        std::ostringstream outputStream;
        this->operator()(outputStream);
        return outputStream.str();
    }

private:
    const T &object_;
    std::string name_;
    int indent_;

};


} // end namespace fields


#define DECLARE_OUTPUT_STREAM_OPERATOR(type)                                \
inline                                                                      \
std::ostream & operator<<(std::ostream &outputStream, const type &value)    \
{                                                                           \
    return outputStream << fields::DescribeCompact(value);                  \
}
