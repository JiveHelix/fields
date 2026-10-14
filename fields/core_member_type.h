#pragma once


#include <fields/field.h>
#include <fields/reflect/reflect.h>


namespace fields
{


template<typename T, IsField F, typename = void>
struct CoreMemberType_
{

};


template<typename T, IsField F>
struct CoreMemberType_
<
    T,
    F,
    std::enable_if_t<IsAddressField<F>>
>
{
    using Type = typename F::Type;
};


template<typename T, IsField F>
struct CoreMemberType_
<
    T,
    F,
    std::enable_if_t<IsIndexField<F>>
>
{
    using Type = typename Reflect<T>::template Element<F::index>;
};


template<typename T, IsField F>
using CoreMemberType = typename CoreMemberType_<T, F>::Type;


template<HasFields T, std::size_t Index>
struct FieldElementType_
{
    using Type =
        CoreMemberType
        <
            T,
            std::tuple_element_t
            <
                Index,
                std::remove_cvref_t<decltype(T::fields)>
            >
        >;
};


template<HasFields T, std::size_t Index>
using FieldElementType =
    typename FieldElementType_<std::remove_cvref_t<T>, Index>::Type;


template<typename Field>
    requires(IsAddressField<std::remove_cvref_t<Field>>)
using FieldType = typename std::remove_reference_t<Field>::Type;



} // end namespace fields
