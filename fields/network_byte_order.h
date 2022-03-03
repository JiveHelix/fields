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

#include "jive/endian_tools.h"
#include "fields/core.h"

namespace fields
{


template<
    typename T,
    typename std::enable_if_t<HasFields<T>::value, int> = 0
>
void HostToNetwork(T &object)
{
    ForEachField<T>(
        [&](auto &field) -> void
        {
            using Type = FieldType<decltype(field)>;

            if constexpr (HasFields<Type>::value)
            {
                HostToNetwork(object.*(field.member));
            }
            else if constexpr (std::is_array_v<Type>)
            {
                using Subtype = std::remove_all_extents_t<Type>;

                if constexpr (HasFields<Subtype>::value)
                {
                    auto begin = jive::Begin(object.*(field.member));
                    auto end = jive::End(object.*(field.member));

                    while (begin != end)
                    {
                        HostToNetwork(*begin);
                        ++begin;
                    }
                }
                else
                {
                    static_assert(
                        std::is_arithmetic_v<std::remove_all_extents_t<Type>>,
                        "Cannot byte-swap non-arithmetic types.");

                    jive::HostToBigEndian(
                        object.*(field.member),
                        object.*(field.member));
                }
            }
            else if constexpr (std::is_arithmetic_v<Type>)
            {
                object.*(field.member) =
                    jive::HostToBigEndian(object.*(field.member));
            }
        });
}


template<
    typename T,
    typename std::enable_if_t<HasFields<T>::value, int> = 0
>
void NetworkToHost(T &object)
{
    ForEachField<T>(
        [&](auto &field) -> void
        {
            using Type = FieldType<decltype(field)>;

            if constexpr (HasFields<Type>::value)
            {
                NetworkToHost(object.*(field.member));
            }
            else if constexpr (std::is_array_v<Type>)
            {
                using Subtype = std::remove_all_extents_t<Type>;

                if constexpr (HasFields<Subtype>::value)
                {
                    auto begin = jive::Begin(object.*(field.member));
                    auto end = jive::End(object.*(field.member));

                    while (begin != end)
                    {
                        NetworkToHost(*begin);
                        ++begin;
                    }
                }
                else
                {
                    static_assert(
                        std::is_arithmetic_v<std::remove_all_extents_t<Type>>,
                        "Cannot byte-swap non-arithmetic types.");

                    jive::BigEndianToHost(
                        object.*(field.member),
                        object.*(field.member));
                }
            }
            else if constexpr (std::is_arithmetic_v<Type>)
            {
                object.*(field.member) =
                    jive::BigEndianToHost(object.*(field.member));
            }
        });
}


} // end namespace fields
