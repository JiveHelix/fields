/**
  * @file comparisons.h
  *
  * @brief Implements comparison operator overloads for fields classes.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 08 Nov 2021
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */

/* To create unqualified comparison overloads, just #include this header.
 * To create namespace-qualified overloads, #include this header inside the
 * relevant namespace.
 *
 * This method will create comparison operators for all classes with a fields
 * tuple. The more targeted approach in fields/compare.h is preferred, for
 * example, DECLARE_COMPARISON_OPERATORS(<your-class-name-here>)
 */

template <typename T>
std::enable_if_t<
    (fields::HasFields<T> && !jive::HasEqualTo<T>::value),
    bool>
operator==(const T &left, const T &right)
{
    return fields::detail::ComparisonTuple(left)
        == fields::detail::ComparisonTuple(right);
}

template <typename T>
std::enable_if_t<
    (fields::HasFields<T> && !jive::HasNotEqualTo<T>::value),
    bool>
operator!=(const T &left, const T &right)
{
    return fields::detail::ComparisonTuple(left)
        != fields::detail::ComparisonTuple(right);
}

template <typename T>
std::
    enable_if_t<(fields::HasFields<T> && !jive::HasLess<T>::value), bool>
    operator<(const T &left, const T &right)
{
    return fields::detail::ComparisonTuple(left)
        < fields::detail::ComparisonTuple(right);
}

template <typename T>
std::enable_if_t<
    (fields::HasFields<T> && !jive::HasGreater<T>::value),
    bool>
operator>(const T &left, const T &right)
{
    return fields::detail::ComparisonTuple(left)
        > fields::detail::ComparisonTuple(right);
}


template<typename T>
std::enable_if_t<
    (fields::HasFields<T> && !jive::HasLessEqual<T>::value),
    bool
>
operator<=(const T &left, const T &right)
{
    return !(left > right);
}


template<typename T>
std::enable_if_t<
    (fields::HasFields<T> && !jive::HasGreaterEqual<T>::value),
    bool
>
operator>=(const T &left, const T &right)
{
    return !(left < right);
}
