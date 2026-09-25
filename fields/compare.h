/**
  * @file compare.h
  *
  * @brief Implements details for comparison operators.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 01 May 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

#pragma once

#include <tuple>
#include <jive/comparison_operators.h>
#include <jive/equal.h>
#include <jive/begin.h>
#include <jive/optional.h>
#include <fields/core.h>
#include <fields/reflect.h>
#include <fields/select.h>


namespace fields
{


template<int precision, CanInspect T>
constexpr auto PrecisionCompare(const T &object);


namespace detail
{


    template<typename, typename = void>
    struct HasPrecision: std::false_type {};

    template<typename T>
    struct HasPrecision
    <
        T,
        std::void_t
        <
            std::enable_if_t
            <
                std::is_convertible_v<decltype(T::precision), int>
            >
        >
    > : std::true_type {};

    template<int precision, typename T>
    bool DoEqual(const T &value, const T &other)
    {
        if constexpr (CanInspect<T>)
        {
            if constexpr (jive::HasMemberEqual<T>)
            {
                return value == other;
            }
            else if constexpr (!HasPrecision<T>::value)
            {
                // There is no precision specified on the inner fields class.
                // Use the precision of the outer container.
                return PrecisionCompare<precision>(value)
                    == PrecisionCompare<precision>(other);
            }
            else
            {
                // Use the operator== of the fields class.
                return value == other;
            }
        }
        else
        {
            if constexpr (precision >= 0)
            {
                return jive::DigitsEqual<
                    T,
                    static_cast<size_t>(precision)>{}(value, other);
            }
            else
            {
                return value == other;
            }
        }
    }

    template<int precision, typename T>
    bool Equal(const T &value, const T &other)
    {
        if constexpr (jive::IsOptional<T>)
        {
            if (!value && !other)
            {
                return true;
            }

            if (value.has_value() != other.has_value())
            {
                return false;
            }

            return DoEqual<precision>(*value, *other);
        }
        else
        {
            return DoEqual<precision>(value, other);
        }
    }

    template<int precision, typename T, typename Enable = void>
    class Compare {};

    template<int precision, typename T>
    class Compare
    <
        precision,
        T,
        typename std::enable_if_t
        <
            std::is_array_v<T>
        >
    >
    {
    public:
        Compare(const T &value): value_(value) {}

        bool operator<(const Compare &other) const
        {
            return std::lexicographical_compare(
                jive::Begin(this->value_),
                jive::End(this->value_),
                jive::Begin(other.value_),
                jive::End(other.value_));
        }

        bool operator>(const Compare &other) const
        {
            return std::lexicographical_compare(
                jive::Begin(this->value_),
                jive::End(this->value_),
                jive::Begin(other.value_),
                jive::End(other.value_),
                std::greater{});
        }

        bool operator==(const Compare &other) const
        {
            using ValueType = std::remove_all_extents_t<T>;

            return std::equal(
                jive::Begin(this->value_),
                jive::End(this->value_),
                jive::Begin(other.value_),
                jive::End(other.value_),
                Equal<precision, ValueType>);
        }

        bool operator!=(const Compare &other) const
        {
            return !(*this == other);
        }

        bool operator<=(const Compare &other) const
        {
            return !(*this > other);
        }

        bool operator>=(const Compare &other) const
        {
            return !(*this < other);
        }

    private:
        const T &value_;
    };


    template<int precision, typename T>
    class Compare
    <
        precision,
        T,
        typename std::enable_if_t
        <
            !std::is_array_v<T>
        >
    >
    {
    public:
        Compare(const T &value): value_(value) {}

        bool operator<(const Compare &other) const
        {
            return this->value_ < other.value_;
        }

        bool operator>(const Compare &other) const
        {
            return this->value_ > other.value_;
        }

        bool operator==(const Compare &other) const
        {
            return Equal<precision>(this->value_, other.value_);
        }

        bool operator!=(const Compare &other) const
        {
            return !(*this == other);
        }

        bool operator<=(const Compare &other) const
        {
            return this->value_ <= other.value_;
        }

        bool operator>=(const Compare &other) const
        {
            return this->value_ >= other.value_;
        }

    private:
        const T &value_;
    };



    template<typename T, typename = void>
    struct Precision
    {
        static constexpr int value = -1;
    };


    template<typename T>
    struct Precision
    <
        T,
        std::void_t
        <
            std::enable_if_t<HasPrecision<T>::value>
        >
    >
    {
        static constexpr int value = T::precision;
    };


    template<int precision, typename T>
    auto MakeCompare(const T &value)
    {
        return Compare<precision, T>(value);
    }

} // end namespace detail


template<int precision, HasFields T, std::size_t... I>
constexpr auto ComparisonTuple(
        const T &object,
        std::index_sequence<I...>)
{
    return std::make_tuple(
        detail::MakeCompare<precision>(
            object.*(std::get<I>(T::fields).member))...);
}


template<int precision, CanReflect T, std::size_t... I>
constexpr auto ComparisonTuple(
        const T &object,
        std::index_sequence<I...>)
{
    return std::make_tuple(
        detail::MakeCompare<precision>(GetMember<I>(object))...);
}


template<CanInspect T>
constexpr auto ComparisonTuple(const T &object)
{
    return ComparisonTuple<detail::Precision<T>::value>(
        object,
        SelectIndices<std::is_empty>(object));
}

template<int precision, CanInspect T>
constexpr auto PrecisionCompare(const T &object)
{
    return ComparisonTuple<precision>(
        object,
        SelectIndices<std::is_empty>(object));
}


} // end namespace fields


#define DECLARE_OPERATOR_EQUALS(Type)              \
    inline bool operator==(const Type &left, const Type &right) \
    {                                                           \
        return fields::ComparisonTuple(left)       \
            == fields::ComparisonTuple(right);     \
    }

#define DECLARE_OPERATOR_NOT_EQUALS(Type)          \
    inline bool operator!=(const Type &left, const Type &right) \
    {                                                           \
        return fields::ComparisonTuple(left)       \
            != fields::ComparisonTuple(right);     \
    }

#define DECLARE_OPERATOR_LESS_THAN(Type)          \
    inline bool operator<(const Type &left, const Type &right) \
    {                                                          \
        return fields::ComparisonTuple(left)      \
            < fields::ComparisonTuple(right);     \
    }

#define DECLARE_OPERATOR_GREATER_THAN(Type)       \
    inline bool operator>(const Type &left, const Type &right) \
    {                                                          \
        return fields::ComparisonTuple(left)      \
            > fields::ComparisonTuple(right);     \
    }

#define DECLARE_OPERATOR_LESS_THAN_EQUALS(Type)    \
    inline bool operator<=(const Type &left, const Type &right) \
    {                                                           \
        return fields::ComparisonTuple(left)       \
            <= fields::ComparisonTuple(right);     \
    }

#define DECLARE_OPERATOR_GREATER_THAN_EQUALS(Type) \
    inline bool operator>=(const Type &left, const Type &right) \
    {                                                           \
        return fields::ComparisonTuple(left)       \
            >= fields::ComparisonTuple(right);     \
    }


#define DECLARE_EQUALITY_OPERATORS(Type)                        \
    DECLARE_OPERATOR_EQUALS(Type)                 \
    DECLARE_OPERATOR_NOT_EQUALS(Type)


#define DECLARE_COMPARISON_OPERATORS(Type)                      \
    DECLARE_EQUALITY_OPERATORS(Type)                            \
    DECLARE_OPERATOR_LESS_THAN(Type)              \
    DECLARE_OPERATOR_GREATER_THAN(Type)           \
    DECLARE_OPERATOR_LESS_THAN_EQUALS(Type)       \
    DECLARE_OPERATOR_GREATER_THAN_EQUALS(Type)


#define TEMPLATE_OPERATOR_EQUALS(Type)                          \
    template <typename T>                                       \
    bool operator==(const Type<T> &left, const Type<T> &right)  \
    {                                                           \
        return fields::ComparisonTuple(left)   \
            == fields::ComparisonTuple(right); \
    }

#define TEMPLATE_OPERATOR_NOT_EQUALS(Type)                      \
    template <typename T>                                       \
    bool operator!=(const Type<T> &left, const Type<T> &right)  \
    {                                                           \
        return fields::ComparisonTuple(left)   \
            != fields::ComparisonTuple(right); \
    }

#define TEMPLATE_OPERATOR_LESS_THAN(Type)                      \
    template <typename T>                                      \
    bool operator<(const Type<T> &left, const Type<T> &right)  \
    {                                                          \
        return fields::ComparisonTuple(left)  \
            < fields::ComparisonTuple(right); \
    }

#define TEMPLATE_OPERATOR_GREATER_THAN(Type)                   \
    template <typename T>                                      \
    bool operator>(const Type<T> &left, const Type<T> &right)  \
    {                                                          \
        return fields::ComparisonTuple(left)  \
            > fields::ComparisonTuple(right); \
    }

#define TEMPLATE_OPERATOR_LESS_THAN_EQUALS(Type)                \
    template <typename T>                                       \
    bool operator<=(const Type<T> &left, const Type<T> &right)  \
    {                                                           \
        return fields::ComparisonTuple(left)   \
            <= fields::ComparisonTuple(right); \
    }

#define TEMPLATE_OPERATOR_GREATER_THAN_EQUALS(Type)             \
    template <typename T>                                       \
    bool operator>=(const Type<T> &left, const Type<T> &right)  \
    {                                                           \
        return fields::ComparisonTuple(left)   \
            >= fields::ComparisonTuple(right); \
    }


#define TEMPLATE_EQUALITY_OPERATORS(Type)                       \
    TEMPLATE_OPERATOR_EQUALS(Type)                              \
    TEMPLATE_OPERATOR_NOT_EQUALS(Type)


#define TEMPLATE_COMPARISON_OPERATORS(Type)     \
    TEMPLATE_EQUALITY_OPERATORS(Type)           \
    TEMPLATE_OPERATOR_LESS_THAN(Type)           \
    TEMPLATE_OPERATOR_GREATER_THAN(Type)        \
    TEMPLATE_OPERATOR_LESS_THAN_EQUALS(Type)    \
    TEMPLATE_OPERATOR_GREATER_THAN_EQUALS(Type)
