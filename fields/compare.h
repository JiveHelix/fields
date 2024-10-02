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
#include "fields/core.h"


namespace fields
{

template<ssize_t precision, typename T>
constexpr auto PrecisionCompare(const T &value);


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
                std::is_convertible_v<decltype(T::precision), ssize_t>
            >
        >
    > : std::true_type {};

    template<ssize_t precision, typename T>
    bool DoEqual(const T &value, const T &other)
    {
        if constexpr (fields::HasFields<T>)
        {
            if constexpr (!HasPrecision<T>::value)
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

    template<ssize_t precision, typename T>
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

    template<ssize_t precision, typename T, typename Enable = void>
    class Compare {};

    template<ssize_t precision, typename T>
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


    template<ssize_t precision, typename T>
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
        static constexpr ssize_t value = -1;
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
        static constexpr ssize_t value = T::precision;
    };


    template<ssize_t precision, typename T>
    auto MakeCompare(const T &value)
    {
        return Compare<precision, T>(value);
    }

    // Build up a list of indices to fields that participate in comparisons.
    template<typename T, typename Fields, size_t Count, size_t... I>
    constexpr auto SelectFields(const T &object, const Fields &fields)
    {
        if constexpr (Count == 0)
        {
            return std::index_sequence<I...>();
        }
        else
        {
            using MemberType = typename std::remove_reference_t
                <
                    decltype(object.*(std::get<Count - 1>(fields).member))
                >;

            if constexpr (std::is_empty_v<MemberType>)
            {
                // Empty types do not participate in comparisons.
                // Skip this field.
                return SelectFields<T, Fields, Count - 1, I...>(object, fields);
            }
            else
            {
                return SelectFields<T, Fields, Count - 1, Count - 1, I...>(
                    object,
                    fields);
            }
        }
    }
} // end namespace detail


template<ssize_t precision, typename T, std::size_t... I>
constexpr auto ComparisonTuple(
        const T &object,
        std::index_sequence<I...>)
{
    return std::make_tuple(
        detail::MakeCompare<precision>(
            object.*(std::get<I>(T::fields).member))...);
}

template<typename T>
constexpr auto ComparisonTuple(const T &object)
{
    static_assert(
        fields::HasFields<T>,
        "Missing required fields tuple");

    using Fields = decltype(T::fields);

    constexpr auto propertyCount = std::tuple_size<Fields>::value;

    return ComparisonTuple<detail::Precision<T>::value>(
        object,
        detail::SelectFields<T, Fields, propertyCount>(object, T::fields));
}

template<ssize_t precision, typename T>
constexpr auto PrecisionCompare(const T &value)
{
    static_assert(
        fields::HasFields<T>,
        "Missing required fields tuple");

    using Fields = decltype(T::fields);

    constexpr auto propertyCount = std::tuple_size<Fields>::value;

    return ComparisonTuple<precision>(
        value,
        detail::SelectFields<T, Fields, propertyCount>(value, T::fields));
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
