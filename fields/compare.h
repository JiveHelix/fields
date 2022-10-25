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
#include "fields/core.h"
#include "jive/comparison_operators.h"
#include "jive/equal.h"
#include "jive/begin.h"

namespace fields
{

namespace detail
{
    template<ssize_t precision, typename T>
    bool Equal(const T& value, const T& other)
    {
        if constexpr (fields::HasFields<T>)
        {
            // Use the operator== of the fields class.
            return value == other;
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

    template<ssize_t precision, typename T, typename Enable = void>
    class Compare {};

    template <ssize_t precision, typename T>
    class Compare<
        precision,
        T,
        typename std::enable_if_t<
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


    template<ssize_t precision, typename T>
    class Compare<
        precision,
        T,
        typename std::enable_if_t<
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

    // Optionally, a class can name itself
    template<typename, typename = void>
    struct HasPrecision: std::false_type {};


    template<typename T>
    struct HasPrecision<
        T,
        std::void_t<
            std::enable_if_t<
                std::is_convertible_v<decltype(T::precision), ssize_t>
            >
        >
    > : std::true_type {};

    template<typename T, typename = void>
    struct Precision
    {
        static constexpr ssize_t value = -1;
    };


    template<typename T>
    struct Precision<
        T,
        std::void_t<
            std::enable_if_t<HasPrecision<T>::value>
        >
    >
    {
        static constexpr ssize_t value = T::precision;
    };


    template<ssize_t precision, typename T>
    auto MakeCompare(const T &value)
    {
        return Compare<precision, T>(value);
    }

    template <typename T, std::size_t... I>
    constexpr auto ComparisonTuple(
            const T &object,
            std::index_sequence<I...>)
    {
        return std::make_tuple(
            MakeCompare<Precision<T>::value>(
                object.*(std::get<I>(T::fields).member))...);
    }

    template <typename T>
    constexpr auto ComparisonTuple(const T &object)
    {
        static_assert(
            fields::HasFields<T>,
            "Missing required fields tuple");

        constexpr auto propertyCount =
            std::tuple_size<decltype(T::fields)>::value;

        return ComparisonTuple(
            object,
            std::make_index_sequence<propertyCount>{});
    }


} // end namespace detail


template <typename T, typename Fields, std::size_t... I>
constexpr auto ComparisonTuple(
    const T &object,
    Fields &&fields,
    std::index_sequence<I...>)
{
    return std::make_tuple(
        detail::MakeCompare<detail::Precision<T>::value>(
            object.*(std::get<I>(fields).member))...);
}

template <typename T, typename Fields>
constexpr auto ComparisonTuple(const T &object, Fields &&fields)
{
    constexpr auto propertyCount =
        std::tuple_size<std::remove_reference_t<decltype(fields)>>::value;

    return ComparisonTuple(
        object,
        std::forward<Fields>(fields),
        std::make_index_sequence<propertyCount>{});
}


} // end namespace fields


#define DECLARE_OPERATOR_EQUALS(Type, fieldsTuple)              \
    inline bool operator==(const Type &left, const Type &right) \
    {                                                           \
        return fields::ComparisonTuple(left, fieldsTuple)       \
            == fields::ComparisonTuple(right, fieldsTuple);     \
    }

#define DECLARE_OPERATOR_NOT_EQUALS(Type, fieldsTuple)          \
    inline bool operator!=(const Type &left, const Type &right) \
    {                                                           \
        return fields::ComparisonTuple(left, fieldsTuple)       \
            != fields::ComparisonTuple(right, fieldsTuple);     \
    }

#define DECLARE_OPERATOR_LESS_THAN(Type, fieldsTuple)          \
    inline bool operator<(const Type &left, const Type &right) \
    {                                                          \
        return fields::ComparisonTuple(left, fieldsTuple)      \
            < fields::ComparisonTuple(right, fieldsTuple);     \
    }

#define DECLARE_OPERATOR_GREATER_THAN(Type, fieldsTuple)       \
    inline bool operator>(const Type &left, const Type &right) \
    {                                                          \
        return fields::ComparisonTuple(left, fieldsTuple)      \
            > fields::ComparisonTuple(right, fieldsTuple);     \
    }

#define DECLARE_OPERATOR_LESS_THAN_EQUALS(Type, fieldsTuple)    \
    inline bool operator<=(const Type &left, const Type &right) \
    {                                                           \
        return fields::ComparisonTuple(left, fieldsTuple)       \
            <= fields::ComparisonTuple(right, fieldsTuple);     \
    }

#define DECLARE_OPERATOR_GREATER_THAN_EQUALS(Type, fieldsTuple) \
    inline bool operator>=(const Type &left, const Type &right) \
    {                                                           \
        return fields::ComparisonTuple(left, fieldsTuple)       \
            >= fields::ComparisonTuple(right, fieldsTuple);     \
    }

#define DECLARE_COMPARISON_OPERATORS(Type, fieldsTuple)  \
    DECLARE_OPERATOR_EQUALS(Type, fieldsTuple)           \
    DECLARE_OPERATOR_NOT_EQUALS(Type, fieldsTuple)       \
    DECLARE_OPERATOR_LESS_THAN(Type, fieldsTuple)        \
    DECLARE_OPERATOR_GREATER_THAN(Type, fieldsTuple)     \
    DECLARE_OPERATOR_LESS_THAN_EQUALS(Type, fieldsTuple) \
    DECLARE_OPERATOR_GREATER_THAN_EQUALS(Type, fieldsTuple)
