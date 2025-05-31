#pragma once


#include <type_traits>


namespace fields
{


// To meet the requirement of a class that implements Fields, a class must
// define a static tuple of Fields describing the class members that will
// participate.
template<typename, typename = void>
struct HasFields_: std::false_type {};

template<typename T>
struct HasFields_<T, std::void_t<decltype(T::fields)>
> : std::true_type {};

template<typename T>
concept HasFields = HasFields_<T>::value;


// Optionally, a class can name itself
template<typename, typename = void>
struct HasFieldsTypeName_: std::false_type {};

template<typename T>
struct HasFieldsTypeName_<T, std::void_t<decltype(T::fieldsTypeName)>
> : std::true_type {};

template<typename T>
inline constexpr bool HasFieldsTypeName = HasFieldsTypeName_<T>::value;


} // end namespace fields
