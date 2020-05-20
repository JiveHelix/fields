/**
  * @file core.h
  * 
  * @brief Core utilities for defining and accessing fields.
  * 
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 01 May 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
  */
#pragma once

#include <type_traits>
#include <optional>
#include <string>
#include "jive/for_each.h"


namespace fields
{

template<typename Class, typename T, typename... OtherNames>
struct Field
{
    using Type = T;

    constexpr Field(
        T Class::*member,
        const char* name,
        OtherNames ...otherNames)
        :
        member{member},
        name{name},
        otherNames{otherNames...}
    {

    }

    T Class::* member;
    const char* name;
    std::tuple<OtherNames...> otherNames;
};


// To meet the requirement of a class that implements Fields, a class must
// define a static tuple of Fields describing the class members that will
// participate.
template<typename, typename = std::void_t<>>
struct HasFields: std::false_type {};


template<typename T>
struct HasFields<T, std::void_t<decltype(T::fields)>
> : std::true_type {};

// Optionally, a class can name itself
template<typename, typename = std::void_t<>>
struct HasFieldsTypeName: std::false_type {};


template<typename T>
struct HasFieldsTypeName<T, std::void_t<decltype(T::fieldsTypeName)>
> : std::true_type {};


template<typename T, typename = std::void_t<>>
struct ImplementsAfterFields: std::false_type {};

template<typename T>
struct ImplementsAfterFields<
    T,
    std::void_t<decltype(std::declval<T>().AfterFields())>
> : std::true_type {};


// Optionally, a class can provide it's own Structure method.
// It must be a static method that takes a single reference to a Json instance.
template<typename T, typename Json, typename = void>
struct ImplementsStructure: std::false_type {};

template<typename T, typename Json>
struct ImplementsStructure<
    T,
    Json,
    std::void_t<decltype(T::Structure(std::declval<Json>()))>
> : std::true_type {};


// Optionally, a class can provide it's own Unstructure method, as a non-static
// class method without arguments, and returning a Json instance.
template<typename T, typename Json, typename = void>
struct ImplementsUnstructure: std::false_type {};

template<typename T, typename Json>
struct ImplementsUnstructure<
    T,
    Json,
    std::void_t<
        std::enable_if_t<
            std::is_same<
                Json,
                decltype(std::declval<T>().template Unstructure<Json>())
            >::value
        >
    >
>: std::true_type {};


template<typename T, typename = std::void_t<>>
struct ImplementsDefault: std::false_type {};

template<typename T>
struct ImplementsDefault<
    T,
    std::void_t<decltype(T::GetDefault())>
> : std::true_type {};


template<std::size_t Index, typename Fields>
using FieldElementType = typename std::tuple_element_t<Index, Fields>::Type;


template<typename Field>
using FieldType = typename std::remove_reference_t<Field>::Type;


template <typename T, std::size_t... I>
constexpr auto GetFields(
        const T &object,
        std::index_sequence<I...>)
{
    return std::tie(object.*(std::get<I>(T::fields).member)...);
}


template <typename T>
constexpr auto GetFields(const T &object)
{
    static_assert(HasFields<T>::value, "Missing required fields tuple");

    constexpr auto propertyCount =
        std::tuple_size<decltype(T::fields)>::value;

    return GetFields(
        object,
        std::make_index_sequence<propertyCount>{});
}


// Call a function for each field
template <typename T, typename F>
constexpr void ForEachField(F &&function)
{
    static_assert(HasFields<T>::value, "Missing required fields tuple");
    jive::ForEach(T::fields, std::forward<F>(function));
}


template<typename Json, typename T>
Json Unstructure(const T &structured);


template<typename Json, typename T>
Json UnstructureFromFields(const T &structured)
{
    Json result;

    ForEachField<T>(
        [&](const auto &field)
        {
            result[field.name] =
                Unstructure<Json>(structured.*(field.member));
        });

    return result;
}


template<typename Json, typename T>
Json Unstructure(const T &structured)
{
    if constexpr (ImplementsUnstructure<T, Json>::value)
    {
        return structured.template Unstructure<Json>();
    }
    else if constexpr (HasFields<T>::value)
    {
        return UnstructureFromFields<Json>(structured);
    }
    else
    {
        // Members without fields or Unstructure method must be convertible to
        // the json value.
        return structured;
    }
}


template<typename Field, typename Json>
const Json * FindMember(const Field &field, const Json &unstructured)
{
    if (1 == unstructured.count(field.name))
    {
        // The canonical name for this field was found.
        return &unstructured[field.name];
    }
    
    auto otherNames = field.otherNames;
    if constexpr (std::tuple_size<decltype(otherNames)>::value > 0)
    {
        std::optional<std::string> matchingName{};

        // The field may exist with a different name.
        jive::ForEach(
            otherNames,
            [&](const char *otherName)
            {
                if (1 == unstructured.count(otherName))
                {
                    matchingName = otherName;
                }
            });
        
        if (matchingName)
        {
            return &unstructured[matchingName.value()];
        }
    }

    // Value not found
    return nullptr;
}


template<typename T>
T Default()
{
    if constexpr (ImplementsDefault<T>::value)
    {
        return T::GetDefault();
    }
    else
    {
        return {};
    }
}


template<typename T, typename Json>
T Structure(const Json &unstructured);


template<typename T, typename Json>
void StructureFromFields(T &result, const Json &unstructured)
{
    if constexpr (HasFields<T>::value)
    {
        // Iterate over fields of T to construct members
        // Any call to Structure on a member that HasFields will end up back
        // here. Members that do not implement fields will fall through to the
        // default initialization below.
        ForEachField<T>(
            [&](const auto &field)
            {
                using Type = FieldType<decltype(field)>;

                auto unstructuredMember = FindMember(field, unstructured);
                    
                if (unstructuredMember)
                {
                    // The field was found.
                    result.*(field.member) =
                        Structure<Type>(*unstructuredMember);
                }
                else
                {
                    // Initialize missing field to its default value
                    result.*(field.member) = Default<Type>();
                }
            });
    }
    else
    {
        // Members without fields must be convertible from the json value.
        result = unstructured;
    }
}


template<typename T, typename Json>
T Structure(const Json &unstructured)
{
    if constexpr (ImplementsStructure<T, Json>::value)
    {
        return T::Structure(unstructured);
    }
    else
    {
        T result;

        StructureFromFields(result, unstructured);

        if constexpr (ImplementsAfterFields<T>::value)
        {
            // Allow T to do any additional initialization.
            result.AfterFields();
        }

        return result;
    }
}


/**
 * If the key exists, the structured value will be returned, otherwise
 * getDefault will be called to create the default value.
 *
 * Using a callable for the default value so you only pay for default
 * initialization when it is needed.
 */
template<typename T, typename Json, typename GetDefault>
T Get(const Json &json, const std::string &key, GetDefault getDefault)
{
    static_assert(std::is_same<T, decltype(getDefault())>::value);

    if (json.count(key) == 1)
    {
        return Structure<T>(json[key]);
    }
    else
    {
        return getDefault();
    }
}


} // end namespace fields
