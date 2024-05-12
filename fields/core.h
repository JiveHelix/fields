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
#include <map>
#include <vector>
#include "jive/for_each.h"
#include "jive/type_traits.h"


namespace fields
{

template<typename Class, typename T, typename... OtherNames>
struct Field
{
    using Type = T;

    constexpr Field(
        T Class::*inMember,
        const char* inName,
        OtherNames ...inOtherNames)
        :
        member{inMember},
        name{inName},
        otherNames{inOtherNames...}
    {

    }

    T Class::* member;
    const char* name;
    std::tuple<OtherNames...> otherNames;
};


// To meet the requirement of a class that implements Fields, a class must
// define a static tuple of Fields describing the class members that will
// participate.
template<typename, typename = void>
struct HasFields_: std::false_type {};

template<typename T>
struct HasFields_<T, std::void_t<decltype(T::fields)>
> : std::true_type {};

template<typename T>
inline constexpr bool HasFields = HasFields_<T>::value;


// Optionally, a class can name itself
template<typename, typename = void>
struct HasFieldsTypeName_: std::false_type {};

template<typename T>
struct HasFieldsTypeName_<T, std::void_t<decltype(T::fieldsTypeName)>
> : std::true_type {};

template<typename T>
inline constexpr bool HasFieldsTypeName = HasFieldsTypeName_<T>::value;


template<typename T, typename = void>
struct ImplementsAfterFields_: std::false_type {};

template<typename T>
struct ImplementsAfterFields_<
    T,
    std::void_t<decltype(std::declval<T>().AfterFields())>
> : std::true_type {};

template<typename T>
inline constexpr bool ImplementsAfterFields = ImplementsAfterFields_<T>::value;


// Optionally, a class can provide it's own Structure method.
// It must be a static method that takes a single reference to a Json instance.
template<typename T, typename Json, typename = void>
struct ImplementsStructure_: std::false_type {};

template<typename T, typename Json>
struct ImplementsStructure_<
    T,
    Json,
    std::void_t<decltype(T::Structure(std::declval<Json>()))>
> : std::true_type {};

template<typename T, typename Json>
inline constexpr bool ImplementsStructure =
    ImplementsStructure_<T, Json>::value;


// Optionally, a class can provide it's own Unstructure method, as a non-static
// class method without arguments, and returning a Json instance.
template<typename T, typename Json, typename = void>
struct ImplementsUnstructure_: std::false_type {};

template<typename T, typename Json>
struct ImplementsUnstructure_<
    T,
    Json,
    std::enable_if_t
    <
        std::is_same_v
        <
            Json,
            decltype(std::declval<T>().template Unstructure<Json>())
        >
    >
>: std::true_type {};

template<typename T, typename Json>
inline constexpr bool ImplementsUnstructure =
    ImplementsUnstructure_<T, Json>::value;


template<typename T, typename = void>
struct ImplementsDefault_: std::false_type {};

template<typename T>
struct ImplementsDefault_<
    T,
    std::void_t<decltype(T::GetDefault())>
> : std::true_type {};

template<typename T>
inline constexpr bool ImplementsDefault = ImplementsDefault_<T>::value;


template<typename T>
struct IsOptional_: std::false_type {};

template<typename T>
struct IsOptional_<std::optional<T>>: std::true_type {};

template<typename T>
inline constexpr bool IsOptional = IsOptional_<T>::value;


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
    static_assert(HasFields<T>, "Missing required fields tuple");

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
    static_assert(HasFields<T>, "Missing required fields tuple");
    jive::ForEach(T::fields, std::forward<F>(function));
}


template<typename Json, typename T>
Json Unstructure(const T &structured);


template<typename T, typename Enable = void>
struct Dimensional {};

template<typename T>
struct Dimensional<
    T,
    std::enable_if_t<std::is_array_v<T>>
>
{
    using type = std::vector<
        typename Dimensional<
            typename std::remove_extent_t<T>
        >::type
    >;
};

template<typename T>
struct Dimensional<
    T,
    std::enable_if_t<!std::is_array_v<T>>
>
{
    using type = T;
};


template<typename T>
typename Dimensional<T>::type UnstructureArray(const T &structured)
{
    static_assert(std::is_array_v<T>, "Must be an array");

    static constexpr size_t size = std::extent_v<T>;

    if constexpr (std::rank_v<T> == 1)
    {
        using Subtype = typename std::remove_extent_t<T>;
        const Subtype * begin = &structured[0];
        const Subtype * end = begin + size;
        std::vector<Subtype> asVector{begin, end};
        return asVector;
    }
    else
    {
        typename Dimensional<T>::type result;

        for (size_t i = 0; i < size; ++i)
        {
            result.push_back(UnstructureArray(structured[i]));
        }

        return result;
    }
}


template<typename Json, typename T>
Json UnstructureFromFields(const T &structured)
{
    Json result;

    ForEachField<T>(
        [&](const auto &field) -> void
        {
            using Type = FieldType<decltype(field)>;

            if constexpr (std::is_array_v<Type>)
            {
                auto asVector = UnstructureArray(structured.*(field.member));
                result[field.name] = Unstructure<Json>(asVector);
            }
            else if constexpr (!std::is_empty_v<Type>)
            {
                result[field.name] =
                    Unstructure<Json>(structured.*(field.member));
            }
        });

    return result;
}


template<typename Json, typename T>
Json Unstructure(const T &structured)
{
    if constexpr (ImplementsUnstructure<T, Json>)
    {
        return structured.template Unstructure<Json>();
    }
    else if constexpr (HasFields<T>)
    {
        return UnstructureFromFields<Json>(structured);
    }
    else if constexpr (jive::IsKeyValueContainer<T>::value)
    {
        // Convert to a map of unstructured json objects.
        std::map<typename T::key_type, Json> result;

        for (auto & [key, value]: structured)
        {
            result[key] = Unstructure<Json>(value);
        }

        return result;
    }
    else if constexpr (jive::IsValueContainer<T>::value)
    {
        // Convert the iterable to a vector of unstructured values.
        std::vector<Json> result;

        for (auto &value: structured)
        {
            result.emplace_back(Unstructure<Json>(value));
        }

        return result;
    }
    else if constexpr (jive::IsBitset<T>::value)
    {
        return structured.to_ullong();
    }
    else if constexpr (IsOptional<T>)
    {
        if (structured)
        {
            return Unstructure<Json>(*structured);
        }

        return {};
    }
    else if constexpr (!std::is_empty_v<T>)
    {
        // All other members must be implicitly convertible to
        // the json value.
        return structured;
    }
    else
    {
        return {};
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
            return &unstructured[*matchingName];
        }
    }

    // Value not found
    return nullptr;
}


template<typename T>
T Default()
{
    if constexpr (ImplementsDefault<T>)
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
void StructureInPlace(T &result, const Json &unstructured)
{
    if constexpr (std::is_array_v<T>)
    {
        static constexpr auto size = std::extent_v<T>;

        for (size_t i = 0; i < size; ++i)
        {
            StructureInPlace(
                result[i],
                unstructured.at(i));
        }
    }
    else if constexpr (!std::is_empty_v<T>)
    {
        result = Structure<T>(unstructured);
    }
}


template<typename T>
void InitializeInPlace(T &result)
{
    if constexpr (std::is_array_v<T>)
    {
        static constexpr auto size = std::extent_v<T>;

        for (size_t i = 0; i < size; ++i)
        {
            InitializeInPlace(result[i]);
        }
    }
    else
    {
        result = Default<T>();
    }
}


template<typename T, typename Json>
void StructureFromFields(T &result, const Json &unstructured)
{
    if constexpr (HasFields<T>)
    {
        // Iterate over fields of T to construct members
        // Any call to Structure on a member that HasFields will end up back
        // here. Members that do not implement fields will fall through to the
        // default initialization below.
        ForEachField<T>(
            [&](const auto &field) -> void
            {
                auto unstructuredMember = FindMember(field, unstructured);

                if (unstructuredMember)
                {
                    // Reconstruct the object from the unstructured data.
                    StructureInPlace(
                        result.*(field.member),
                        *unstructuredMember);
                }
                else
                {
                    // Initialize missing field to its default value
                    InitializeInPlace(result.*(field.member));
                }
            });
    }
    else if constexpr (jive::IsKeyValueContainer<T>::value)
    {
        // Convert the key value pairs to the map type.
        auto asMap =
            unstructured.template get<std::map<typename T::key_type, Json>>();

        for (auto & [key, value]: asMap)
        {

            result[key] = Structure<typename T::mapped_type>(value);
        }
    }
    else if constexpr (jive::IsValueContainer<T>::value)
    {
        for (auto &value: unstructured)
        {
            result.push_back(Structure<typename T::value_type>(value));
        }
    }
    else if constexpr (IsOptional<T>)
    {
        using ValueType = typename T::value_type;

        if (!unstructured.is_null())
        {
            result = Structure<ValueType>(unstructured);
        }
    }
    else
    {
        if constexpr (jive::IsString<T>::value)
        {
            result = static_cast<std::string>(unstructured);
        }
        else if constexpr (jive::IsBitset<T>::value)
        {
            result = static_cast<unsigned long long>(unstructured);
        }
        else if constexpr (!std::is_empty_v<T>)
        {
            // Other members without fields must be convertible from the json
            // value.
            result = unstructured;
        }
    }
}


template<typename T, typename Json>
T Structure(const Json &unstructured)
{
    if constexpr (ImplementsStructure<T, Json>)
    {
        return T::Structure(unstructured);
    }
    else
    {
        T result;

        StructureFromFields(result, unstructured);

        if constexpr (ImplementsAfterFields<T>)
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
    static_assert(std::is_same_v<T, decltype(getDefault())>);

    if (json.count(key) == 1)
    {
        return Structure<T>(json[key]);
    }

    return getDefault();
}


/***** Identity *****/
template<typename T>
using Identity = T;


#define DECLARE_ADAPTERS(FieldsClass, Adapted)                       \
    FieldsClass() = default;                                         \
                                                                     \
    FieldsClass(const Adapted &other)                                \
    {                                                                \
        this->operator=(other);                                      \
    }                                                                \
                                                                     \
    Adapted Get() const                                              \
    {                                                                \
        static_assert(                                               \
            !std::is_polymorphic_v<FieldsClass>,                     \
            "Polymorphism breaks alignment of the 'this' pointer."); \
                                                                     \
        static_assert(sizeof(Adapted) == sizeof(FieldsClass));       \
        Adapted result;                                              \
        memcpy(&result, this, sizeof(Adapted));                      \
        return result;                                               \
    }                                                                \
                                                                     \
    FieldsClass &operator=(const Adapted &other)                     \
    {                                                                \
        static_assert(                                               \
            !std::is_polymorphic_v<FieldsClass>,                     \
            "Polymorphism breaks alignment of the 'this' pointer."); \
                                                                     \
        static_assert(sizeof(Adapted) == sizeof(FieldsClass));       \
        memcpy(this, &other, sizeof(Adapted));                       \
        return *this;                                                \
    }


} // end namespace fields
