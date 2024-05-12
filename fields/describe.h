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
struct DescribeType<T, std::enable_if_t<fields::HasFieldsTypeName<T>>>
{
    static constexpr std::string_view value = T::fieldsTypeName;
};

} // end namespace jive


namespace fields
{


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


struct Colors
{
    const char *name;
    const char *structure;
    const char *type;

    Colors()
        :
        name(DefaultColors::name),
        structure(DefaultColors::structure),
        type(DefaultColors::type)
    {

    }

    template<typename T>
    static Colors Create()
    {
        Colors result;
        result.name = T::name;
        result.structure = T::structure;
        result.type = T::type;

        return result;
    }

    Colors & Name(const char *name_)
    {
        this->name = name_;
        return *this;
    }

    Colors & Structure(const char *structure_)
    {
        this->structure = structure_;
        return *this;
    }

    Colors & Type(const char *type_)
    {
        this->type = type_;
        return *this;
    }
};


struct Style
{
    Colors colors;
    bool verbose;

    Style()
        :
        colors(),
        verbose(false)
    {

    }

    Style(const Colors &colors_, bool verbose_)
        :
        colors(colors_),
        verbose(verbose_)
    {

    }
};


/** Optionally, a class can provide it's own Describe method.
 ** Must be a member function like
 **
 ** std::ostream & Describe(std::ostream &, const Style &, int) const;
 **
 **/

template<typename T, typename = void>
struct HasDescribe_: std::false_type {};

template<typename T>
struct HasDescribe_
<
    T,
    std::enable_if_t
    <
        std::is_same_v
        <
            std::ostream &,
            decltype(
                std::declval<T>().Describe(
                    std::declval<std::ostream &>(),
                    std::declval<Style>(),
                    std::declval<int>()))
        >
    >
>: std::true_type {};


template<typename T>
inline constexpr bool HasDescribe = HasDescribe_<T>::value;


template<typename T, typename = void>
struct HasDoDescribe_: std::false_type {};

template<typename T>
struct HasDoDescribe_
<
    T,
    std::enable_if_t<
        std::is_same_v<
            std::ostream &,
            decltype(
                DoDescribe(
                    std::declval<std::ostream &>(),
                    std::declval<T>(),
                    std::declval<Style>(),
                    std::declval<int>()))
        >
    >
>: std::true_type {};


template<typename T>
inline constexpr bool HasDoDescribe = HasDoDescribe_<T>::value;


template<typename T, typename = void>
struct CanDescribe_: std::false_type {};


template<typename T>
struct CanDescribe_
<
    T,
    std::enable_if_t
    <
        HasFields<T>
        || HasDescribe<T>
        || HasDoDescribe<T>
    >
>: std::true_type {};

template<typename T>
inline constexpr bool CanDescribe = CanDescribe_<T>::value;


template<
    typename T,
    typename Colors = DefaultColors,
    typename VerboseTypes = std::false_type>
class Describe;


template<typename T, typename Colors = DefaultColors>
auto DescribeColorized(const T &object, int indent = -1)
{
    static_assert(
        CanDescribe<T>,
        "Type cannot be described");

    return Describe<T, Colors>(object, indent);
}


template<typename T, typename Colors = DefaultColors>
auto DescribeColorizedVerbose(const T &object, int indent = -1)
{
    static_assert(
        CanDescribe<T>,
        "Type cannot be described");

    return Describe<T, Colors, std::true_type>(object, indent);
}


template<typename T, typename Colors = DefaultColors>
auto DescribeCompact(const T &object, int indent = -1)
{
    static_assert(
        CanDescribe<T>,
        "Type cannot be described");

    return Describe<T, Colors, std::false_type>(object, indent);
}


template<typename T, typename Colors, typename VerboseTypes>
std::string ToString(const Describe<T, Colors, VerboseTypes> &describe)
{
    static_assert(
        CanDescribe<T>,
        "Type cannot be described");

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


#if defined(__GNUG__) && !defined(__clang__) && !defined(_WIN32)
// Avoid bogus -Wrestrict
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105651
#ifndef DO_PRAGMA
#define DO_PRAGMA_(arg) _Pragma (#arg)
#define DO_PRAGMA(arg) DO_PRAGMA_(arg)
#endif

#define GNU_NO_RESTRICT_PUSH \
    DO_PRAGMA(GCC diagnostic push); \
    DO_PRAGMA(GCC diagnostic ignored "-Wrestrict");

#define GNU_NO_RESTRICT_POP \
    DO_PRAGMA(GCC diagnostic pop);

#else

#define GNU_NO_RESTRICT_PUSH
#define GNU_NO_RESTRICT_POP

#endif // defined __GNUG__


inline std::string MakeIndent(int indent)
{
    if (indent <= 0)
    {
        return {};
    }

GNU_NO_RESTRICT_PUSH

    return "\n" + std::string(static_cast<unsigned>(indent) * 4, ' ');

GNU_NO_RESTRICT_POP

}


template<typename Object, typename Fields, std::size_t... I>
std::ostream & DescribeFields(
    std::ostream &outputStream,
    const Object &object,
    const Fields &fields,
    const Style &style,
    int indent,
    std::index_sequence<I...>)
{
    // Use a fold expression to print a comma between each member.
    // Don't print a comma before the first member.
    ((outputStream << (I == 0 ? "" : ", ") <<
        /* Recursively wrap each member in Describe */
        Describe<FieldElementType<I, Fields>>(
            object.*(std::get<I>(fields).member),
            std::get<I>(fields).name,
            (indent < 0) ? -1 : indent + 1)
                .Style(style)),
     ...);

    return outputStream;
}

template<typename Object, typename Fields>
std::ostream & DescribeFields(
    std::ostream &outputStream,
    const Object &object,
    const Fields &fields,
    const Style &style,
    int indent)
{
    constexpr auto itemCount = std::tuple_size<Fields>::value;

    jive::Colorize colorize(outputStream);
    colorize(style.colors.structure, jive::GetTypeName<Object>());
    outputStream << "(";

    DescribeFields(
        outputStream,
        object,
        fields,
        style,
        indent,
        std::make_index_sequence<itemCount>{});

    return outputStream << ")";
}





template<typename T, typename ColorsType, typename VerboseTypes>
class Describe
{
public:
    Describe(const T &object, const std::string &name, int indent = -1)
        :
        object_{object},
        name_{name},
        indent_{indent},
        style_(Colors::Create<ColorsType>(), VerboseTypes::value)
    {

    }

    Describe(const T &object, int indent = -1)
        :
        object_{object},
        name_{},
        indent_{indent},
        style_(Colors::Create<ColorsType>(), VerboseTypes::value)
    {

    }

    // We are storing a const reference. Disallow temporaries.
    Describe(T &&object, const std::string &name) = delete;

    Describe & Colors(const Colors &colors)
    {
        this->style_.colors = colors;
        return *this;
    }

    Describe & Verbose(bool verbose)
    {
        this->style_.verbose = verbose;
        return *this;
    }

    Describe & Style(const Style &options)
    {
        this->style_ = options;
        return *this;
    }


    std::string GetIndent() const
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            if (this->name_.empty())
            {
                // Print un-named numeric values on the same line.
                return {};
            }
        }

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
            outputStream << Describe<typename Object::mapped_type>(
                value,
                KeyToString(key),
                (this->indent_ < 0) ? -1 : this->indent_ + 1)
                    .Style(this->style_);

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
            outputStream << Describe<typename Object::value_type>(
                value,
                std::to_string(count),
                (this->indent_ < 0) ? -1 : this->indent_ + 1)
                    .Style(this->style_);

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
            Describe<Object>(
                array[I],
                (this->indent_ < 0) ? -1 : this->indent_ + 1)
                    .Style(this->style_)
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

    std::ostream & operator()(std::ostream &outputStream) const
    {
        jive::Colorize colorize(outputStream);

        outputStream << this->GetIndent();

        if (!this->name_.empty())
        {
            colorize(this->style_.colors.name, this->name_, ": ");
        }

        if constexpr (HasDescribe<T>)
        {
            this->object_.Describe(
                outputStream,
                this->style_,
                (this->indent_ < 0) ? -1 : this->indent_ + 1);
        }
        else if constexpr (HasDoDescribe<T>)
        {
            DoDescribe(
                outputStream,
                this->object_,
                this->style_,
                (this->indent_ < 0) ? -1 : this->indent_ + 1);
        }
        else if constexpr (HasFields<T>)
        {
            DescribeFields(
                outputStream,
                this->object_,
                T::fields,
                this->style_,
                this->indent_);
        }
        else
        {
            // There are no fields classes, so get a string representation of
            // the object directly.
            if (this->style_.verbose)
            {
                colorize(this->style_.colors.type, jive::GetTypeName<T>());
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
            else if constexpr (jive::HasOutputStreamOperator<T>::value)
            {
                outputStream << this->object_;
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
                        Describe
                        <
                            std::remove_const_t<std::remove_pointer_t<T>>
                        >(
                            *(this->object_),
                            (this->indent_ < 0) ? -1 : this->indent_ + 1)
                                .Style(this->style_);
                }
            }
            else if constexpr (IsOptional<T>)
            {
                using ValueType = typename T::value_type;

                if (this->object_)
                {
                    outputStream <<
                        Describe<ValueType>(
                            *(this->object_),
                            (this->indent_ < 0) ? -1 : this->indent_ + 1)
                                .Style(this->style_);
                }
                else
                {
                    outputStream << "None";
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
    struct Style style_;

};


} // end namespace fields


#define DECLARE_OUTPUT_STREAM_OPERATOR(type)                                \
inline                                                                      \
std::ostream & operator<<(std::ostream &outputStream, const type &value)    \
{                                                                           \
    return outputStream << fields::DescribeCompact(value);                  \
}


#define TEMPLATE_OUTPUT_STREAM(Type)                                        \
template<typename T>                                                        \
std::ostream & operator<<(std::ostream &outputStream, const Type<T> &value) \
{                                                                           \
    return outputStream << fields::DescribeCompact(value);                  \
}
