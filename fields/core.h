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

#include <string>
#include <map>
#include <vector>
#include <jive/for_each.h>
#include <jive/optional.h>
#include <jive/type_traits.h>
#include <fields/field.h>
#include <fields/aliases.h>
#include <fields/has_fields.h>
#include <fields/for_each.h>
#include <fields/reflect.h>
#include <fields/hidden.h>


namespace fields
{


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


/** Conversion from enum to string and back **/

template<typename T>
concept HasToString = requires(T t)
{
    { ToString(t) } -> std::convertible_to<std::string>;
};


template<typename E> struct Tag { using type = E; };

template<typename T>
concept HasToValue = requires
{
    { ToValue(Tag<T>{}, std::declval<std::string>()) }
        -> std::convertible_to<T>;
};


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


template<std::size_t Index, typename Fields>
using FieldElementType = typename std::tuple_element_t<Index, Fields>::Type;


template<typename Field>
using FieldType = typename std::remove_reference_t<Field>::Type;


template<size_t Index, typename T>
    requires HasFields<std::remove_cvref_t<T>>
decltype(auto) GetMember(T &&t)
{
    using U = std::remove_cvref_t<T>;
    return t.*(std::get<Index>(U::fields).member);
}


template<typename T>
struct MemberCount_;

template<typename T>
    requires HasFields<std::remove_cvref_t<T>>
struct MemberCount_<T>
{
    static constexpr size_t value = std::tuple_size_v<decltype(T::fields)>;
};

template<typename T>
    requires CanReflect<std::remove_cvref_t<T>>
struct MemberCount_<T>
{
    static constexpr size_t value = GetMemberCount<T>();
};

template<typename T>
inline constexpr size_t MemberCount = MemberCount_<T>::value;


template<typename Json, typename T>
Json Unstructure(const T &structured);


// Creates a std::vector<T> for 1-dim arrays,
// Creates a std::vector<std::vector<T>> for 2-dim arrays, etc.
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


template<typename Json, HasFields T>
Json UnstructureFromFields(const T &structured)
{
    Json result;

    ForEachFieldIndexed<T>(
        [&]<size_t fieldIndex>() -> void
        {
            constexpr const auto &field = std::get<fieldIndex>(T::fields);

            if constexpr (!GetIsHiddenField<T>(field))
            {
                using Type = FieldType<decltype(field)>;

                if constexpr (std::is_array_v<Type>)
                {
                    auto asVector =
                        UnstructureArray(structured.*(field.member));

                    result[field.name] = Unstructure<Json>(asVector);
                }
                else if constexpr (!std::is_empty_v<Type>)
                {
                    result[field.name] =
                        Unstructure<Json>(structured.*(field.member));
                }
            }
        });

    return result;
}


template<typename Json, typename T, IsFieldsTuple Fields>
Json UnstructureFromFields(const T &structured, const Fields &fields)
{
    Json result;

    jive::ForEach(
        fields,
        [&](const auto &field) -> void
        {
            if (!GetIsHiddenField<T>(field))
            {
                using Type = FieldType<decltype(field)>;

                if constexpr (std::is_array_v<Type>)
                {
                    auto asVector =
                        UnstructureArray(structured.*(field.member));

                    result[field.name] = Unstructure<Json>(asVector);
                }
                else if constexpr (!std::is_empty_v<Type>)
                {
                    result[field.name] =
                        Unstructure<Json>(structured.*(field.member));
                }
            }
        });

    return result;
}


template<typename Json, CanReflect T>
Json UnstructureFromReflection(const T &structured)
{
    Json result;

    ForEachIndexed(
        structured,
        [&result]<size_t memberIndex>(
            const auto &fieldNames,
            const auto &member)
        {
            using Member = std::remove_cvref_t<decltype(member)>;

            if constexpr (!GetExcludeHidden<T, Member, memberIndex>())
            {
                if constexpr (std::is_array_v<Member>)
                {
                    auto asVector = UnstructureArray(member);
                    result[fieldNames.name] = Unstructure<Json>(asVector);
                }
                else if constexpr (!std::is_empty_v<Member>)
                {
                    result[fieldNames.name] = Unstructure<Json>(member);
                }
            }
        });

    return result;
}


template<typename Json, typename T>
Json Unstructure(const T &structured)
{
    if constexpr (std::is_same_v<T, Json>)
    {
        return structured;
    }
    else if constexpr (ImplementsUnstructure<T, Json>)
    {
        return structured.template Unstructure<Json>();
    }
    else if constexpr (HasFields<T>)
    {
        return UnstructureFromFields<Json>(structured);
    }
    else if constexpr (!jive::IsArray<T> && CanReflect<T>)
    {
        return UnstructureFromReflection<Json>(structured);
    }
    else if constexpr (jive::IsKeyValueContainer<T>)
    {
        // Convert to a map of unstructured json objects.
        std::map<typename T::key_type, Json> result;

        for (auto & [key, value]: structured)
        {
            result[key] = Unstructure<Json>(value);
        }

        return result;
    }
    else if constexpr (jive::IsValueContainer<T> || jive::IsArray<T>)
    {
        // Convert the iterable to a vector of unstructured values.
        std::vector<Json> result;

        for (auto &value: structured)
        {
            result.emplace_back(Unstructure<Json>(value));
        }

        return result;
    }
    else if constexpr (jive::IsBitset<T>)
    {
        return structured.to_ullong();
    }
    else if constexpr (jive::IsOptional<T>)
    {
        if (structured)
        {
            return Unstructure<Json>(*structured);
        }

        return {};
    }
    else if constexpr (std::is_enum_v<T>)
    {
        if constexpr (HasToString<T>)
        {
            return ToString(structured);
        }
        else
        {
            return structured;
        }
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
        // The default name for this field was found.
        return &unstructured[field.name];
    }

    auto otherNames = field.otherNames;

    if constexpr (std::tuple_size<decltype(otherNames)>::value > 0)
    {
        std::optional<std::string_view> matchingName{};

        // The field may exist with a different name.
        jive::ForEach(
            otherNames,
            [&](std::string_view otherName)
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


template<typename T, typename Json>
T Restructure(const Json &unstructured)
{
    T result{};

    if constexpr (HasFields<T>)
    {
        // Iterate over fields of T to construct members
        // Any call to Structure on a member that HasFields will end up back
        // here. Members that do not implement fields will fall through to the
        // default initialization below.
        ForEachFieldIndexed<T>(
            [&]<size_t fieldIndex>() -> void
            {
                constexpr const auto &field = std::get<fieldIndex>(T::fields);

                if constexpr (!GetIsHiddenField<T>(field))
                {
                    auto unstructuredMember = FindMember(field, unstructured);

                    if (unstructuredMember)
                    {
                        // Reconstruct the object from the unstructured data.
                        StructureInPlace(
                            result.*(field.member),
                            *unstructuredMember);
                    }
                }
            });
    }
    else if constexpr (!jive::IsArray<T> && CanReflect<T>)
    {
        ForEachIndexed(
            result,
            [&unstructured]<size_t memberIndex>(
                const auto &fieldNames,
                auto &member) -> void
            {
                using Member = std::remove_cvref_t<decltype(member)>;

                if constexpr (!GetExcludeHidden<T, Member, memberIndex>())
                {
                    auto unstructuredMember =
                        FindMember(fieldNames, unstructured);

                    if (unstructuredMember)
                    {
                        StructureInPlace(
                            member,
                            *unstructuredMember);
                    }
                }
            });
    }
    else if constexpr (jive::IsKeyValueContainer<T>)
    {
        // Convert the key value pairs to the map type.
        auto asMap =
            unstructured.template get<std::map<typename T::key_type, Json>>();

        for (auto & [key, value]: asMap)
        {

            result[key] = Structure<typename T::mapped_type>(value);
        }
    }
    else if constexpr (jive::IsValueContainer<T>)
    {
        for (auto &value: unstructured)
        {
            result.push_back(Structure<typename T::value_type>(value));
        }
    }
    else if constexpr (jive::IsArray<T>)
    {
        for (size_t i = 0; i < unstructured.size(); ++i)
        {
            result[i] = Structure<typename T::value_type>(unstructured[i]);
        }
    }
    else if constexpr (jive::IsOptional<T>)
    {
        using ValueType = typename T::value_type;

        if (!unstructured.is_null())
        {
            result = Structure<ValueType>(unstructured);
        }
    }
    else if constexpr (std::is_enum_v<T>)
    {
        if constexpr (HasToValue<T>)
        {
            result = ToValue(Tag<T>{}, std::string(unstructured));
        }
        else
        {
            result = unstructured;
        }
    }
    else
    {
        if constexpr (jive::IsString<T>)
        {
            result = static_cast<std::string>(unstructured);
        }
        else if constexpr (jive::IsBitset<T>)
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

    if constexpr (ImplementsAfterFields<T>)
    {
        // Allow T to do any additional initialization.
        result.AfterFields();
    }

    return result;
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
        return Restructure<T>(unstructured);
    }
}


/***** Identity *****/
template<typename T>
using Identity = T;


} // end namespace fields
