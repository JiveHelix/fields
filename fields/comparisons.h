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
 */

template <typename T>
std::enable_if_t<
    (fields::HasFields<T>::value && !jive::HasEqualTo<T>::value),
    bool>
operator==(const T &left, const T &right)
{
    return fields::detail::ComparisonTuple(left)
        == fields::detail::ComparisonTuple(right);
}

template <typename T>
std::enable_if_t<
    (fields::HasFields<T>::value && !jive::HasNotEqualTo<T>::value),
    bool>
operator!=(const T &left, const T &right)
{
    return fields::detail::ComparisonTuple(left)
        != fields::detail::ComparisonTuple(right);
}

template <typename T>
std::
    enable_if_t<(fields::HasFields<T>::value && !jive::HasLess<T>::value), bool>
    operator<(const T &left, const T &right)
{
    return fields::detail::ComparisonTuple(left)
        < fields::detail::ComparisonTuple(right);
}

template <typename T>
std::enable_if_t<
    (fields::HasFields<T>::value && !jive::HasGreater<T>::value),
    bool>
operator>(const T &left, const T &right)
{
    return fields::detail::ComparisonTuple(left)
        > fields::detail::ComparisonTuple(right);
}


template<typename T>
std::enable_if_t<
    (fields::HasFields<T>::value && !jive::HasLessEqual<T>::value),
    bool
>
operator<=(const T &left, const T &right)
{
    return !(left > right);
}


template<typename T>
std::enable_if_t<
    (fields::HasFields<T>::value && !jive::HasGreaterEqual<T>::value),
    bool
>
operator>=(const T &left, const T &right)
{
    return !(left < right);
}
