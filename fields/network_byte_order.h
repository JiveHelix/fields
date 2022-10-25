/**
  * @file network_byte_order.h
  *
  * @brief Swap all fields to and from network byte order.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 03 Sep 2021
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <cstring>
#include <array>
#include <jive/begin.h>
#include <jive/endian_tools.h>

#include "fields/core.h"


namespace fields
{


namespace detail
{


template<typename, typename = std::void_t<>>
struct HasNetworkMembers_: std::false_type {};

template<typename T>
struct HasNetworkMembers_<T, std::void_t<decltype(T::networkMembers)>
> : std::true_type {};


} // end namespace detail


// To use HostToNetwork and NetworkToHost, a class must define either a fields
// tuple or a networkMembers tuple. networkMembers should be a subset of
// fields, including only those members that participate in byte order
// swapping.
template<typename T>
inline constexpr bool HasNetworkMembers = detail::HasNetworkMembers_<T>::value;


/*** HostToNetwork ***/

template
<
    typename T,
    typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0
>
void HostToNetwork(T &value)
{
    value = jive::HostToBigEndian(value);
}


// Forward declarations.
template
<
    typename T,
    typename std::enable_if_t<std::is_array_v<T>, int> = 0
>
void HostToNetwork(T &);


template
<
    typename T,
    typename std::enable_if_t<HasNetworkMembers<T>, int> = 0
>
void HostToNetwork(T &);


template
<
    typename T,
    typename std::enable_if_t<HasFields<T> && !HasNetworkMembers<T>, int> = 0
>
void HostToNetwork(T &);


// Implementations
template
<
    typename T,
    typename std::enable_if_t<std::is_array_v<T>, int>
>
void HostToNetwork(T &value)
{
    auto begin = jive::Begin(value);
    auto end = jive::End(value);

    while (begin != end)
    {
        HostToNetwork(*begin);
        ++begin;
    }
}


template
<
    typename T,
    typename std::enable_if_t<HasNetworkMembers<T>, int>
>
void HostToNetwork(T &object)
{
    jive::ForEach(
        T::networkMembers,
        [&](auto &field)
        {
            HostToNetwork(object.*(field.member));
        });
}


template
<
    typename T,
    typename std::enable_if_t<HasFields<T> && !HasNetworkMembers<T>, int>
>
void HostToNetwork(T &object)
{
    ForEachField<T>(
        [&](auto &field) -> void
        {
            HostToNetwork(object.*(field.member));
        });
}


/*** NetworkToHost ***/

template
<
    typename T,
    typename std::enable_if_t<std::is_arithmetic_v<T>, int> = 0
>
void NetworkToHost(T &value)
{
    value = jive::BigEndianToHost(value);
}


// Forward declarations
template
<
    typename T,
    typename std::enable_if_t<std::is_array_v<T>, int> = 0
>
void NetworkToHost(T &);


template
<
    typename T,
    typename std::enable_if_t<HasNetworkMembers<T>, int> = 0
>
void NetworkToHost(T &);


template
<
    typename T,
    typename std::enable_if_t<HasFields<T> && !HasNetworkMembers<T>, int> = 0
>
void NetworkToHost(T &);


template
<
    typename T,
    typename std::enable_if_t<std::is_array_v<T>, int>
>
void NetworkToHost(T &value)
{
    auto begin = jive::Begin(value);
    auto end = jive::End(value);

    while (begin != end)
    {
        NetworkToHost(*begin);
        ++begin;
    }
}


template
<
    typename T,
    typename std::enable_if_t<HasNetworkMembers<T>, int>
>
void NetworkToHost(T &object)
{
    jive::ForEach(
        T::networkMembers,
        [&](auto &field)
        {
            NetworkToHost(object.*(field.member));
        });
}


template
<
    typename T,
    typename std::enable_if_t<HasFields<T> && !HasNetworkMembers<T>, int>
>
void NetworkToHost(T &object)
{
    ForEachField<T>(
        [&](auto &field) -> void
        {
            NetworkToHost(object.*(field.member));
        });
}


template<typename T, size_t N>
void ToNetworkBytes(const T &object, std::array<uint8_t, N> &data)
{
    static_assert(N >= sizeof(T));

    T result{object};
    HostToNetwork(result);
    std::memcpy(&data[0], &result, sizeof(T));
}


template<typename T, size_t N>
T FromNetworkBytes(const std::array<uint8_t, N> &data)
{
    static_assert(N >= sizeof(T));

    T result;
    std::memcpy(&result, &data[0], sizeof(T));
    NetworkToHost(result);
    return result;
}


} // end namespace fields
