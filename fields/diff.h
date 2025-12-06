#pragma once


#include <fields/core.h>


namespace fields
{



template<typename Json, typename T>
std::optional<Json> Diff(const T &structured, const T &compare);


// Optionally, a class can provide it's own Diff method, as a non-static
// class method without arguments, and returning a Json instance.
template<typename T, typename Json, typename = void>
struct ImplementsDiff_: std::false_type {};

template<typename T, typename Json>
struct ImplementsDiff_<
    T,
    Json,
    std::enable_if_t
    <
        std::is_same_v
        <
            std::optional<Json>,
            decltype(std::declval<T>().template Diff<Json>(std::declval<T>()))
        >
    >
>: std::true_type {};

template<typename T, typename Json>
inline constexpr bool ImplementsDiff =
    ImplementsDiff_<T, Json>::value;



// Allows for sparse representation of array differences.
// Creates a std::map<size_t, T> for 1-dim arrays,
// Creates a std::map<size_t, std::map<size_t, T>> for 2-dim arrays, etc.
template<typename Json, typename T, typename Enable = void>
struct DimensionalDiff_ {};

template<typename Json, typename T>
struct DimensionalDiff_<
    Json,
    T,
    std::enable_if_t<std::is_array_v<T>>
>
{
    using type =
        std::map
        <
            std::string,
            typename
                DimensionalDiff_
                <
                    Json,
                    typename std::remove_extent_t<T>
                >
                ::type
        >;
};

template<typename Json, typename T>
struct DimensionalDiff_<
    Json,
    T,
    std::enable_if_t<!std::is_array_v<T>>
>
{
    using type = Json;
};


template<typename Json, typename T>
using DimensionalDiff = typename DimensionalDiff_<Json, T>::type;



template<typename Json, typename T>
DimensionalDiff<Json, T> DiffArray(const T &structured, const T &compare)
{
    static_assert(std::is_array_v<T>, "Must be an array");

    static constexpr size_t size = std::extent_v<T>;
    DimensionalDiff<Json, T> result;

    if constexpr (std::rank_v<T> == 1)
    {
        for (size_t i = 0; i < size; ++i)
        {
            auto diff = Diff<Json>(structured[i], compare[i]);

            if (diff)
            {
                result[std::to_string(i)] = *diff;
            }
        }
    }
    else
    {
        for (size_t i = 0; i < size; ++i)
        {
            auto diff = DiffArray<Json>(structured[i], compare[i]);

            if (!diff.empty())
            {
                result[std::to_string(i)] = diff;
            }
        }
    }

    return result;
}


template<typename Json, HasFields T>
std::optional<Json> DiffFromFields(const T &structured, const T &compare)
{
    Json result;

    ForEachField<T>(
        [&](const auto &field) -> void
        {
            using Type = FieldType<decltype(field)>;

            if constexpr (std::is_array_v<Type>)
            {
                auto asMap =
                    DiffArray<Json>(
                        structured.*(field.member),
                        compare.*(field.member));

                if (!asMap.empty())
                {
                    result[field.name] = Unstructure<Json>(asMap);
                }
            }
            else if constexpr (!std::is_empty_v<Type>)
            {
                auto diff = Diff<Json>(
                    structured.*(field.member),
                    compare.*(field.member));

                if (diff)
                {
                    result[field.name] = *diff;
                }
            }
        });

    if (result.empty())
    {
        return {};
    }

    return result;
}


template<typename Json, CanReflect T>
std::optional<Json> DiffFromReflection(const T &structured, const T &compare)
{
    Json result;

    ForEachZip(
        structured,
        compare,
        [&result](
            const auto &name,
            const auto &structuredMember,
            const auto &compareMember)
        {
            using Type = decltype(structuredMember);

            if constexpr (std::is_array_v<Type>)
            {
                auto asMap = DiffArray<Json>(structuredMember, compareMember);

                if (!asMap.empty())
                {
                    result[name] = Unstructure<Json>(asMap);
                }
            }
            else if constexpr (!std::is_empty_v<Type>)
            {
                auto diff = Diff<Json>(structuredMember, compareMember);

                if (diff)
                {
                    result[name] = *diff;
                }
            }
        });

    if (result.empty())
    {
        return {};
    }

    return result;
}


template<typename Json, typename T>
std::optional<Json> Diff(const T &structured, const T &compare)
{
    if constexpr (ImplementsDiff<T, Json>)
    {
        return structured.template Diff<Json>(compare);
    }
    else if constexpr (HasFields<T>)
    {
        return DiffFromFields<Json>(structured, compare);
    }
    else if constexpr (!jive::IsArray<T> && CanReflect<T>)
    {
        return DiffFromReflection<Json>(structured, compare);
    }
    else if constexpr (jive::IsKeyValueContainer<T>::value)
    {
        // Convert to a map of unstructured json objects.
        std::map<typename T::key_type, Json> result;

        for (auto & [key, value]: structured)
        {
            if (!compare.count(key))
            {
                result[key] = Unstructure<Json>(value);
            }
            else
            {
                auto diff = Diff<Json>(value, compare.at(key));

                if (diff)
                {
                    result[key] = *diff;
                }
            }
        }

        for (auto & [key, value]: compare)
        {
            if (!structured.count(key))
            {
                // This value was in compare, but has been removed in
                // structured.
                // Set it to a json null to indicate that it should be removed.
                Json none = nullptr;
                result[key] = none;
            }
        }

        if (result.empty())
        {
            return {};
        }

        return result;
    }
    else if constexpr (jive::IsValueContainer<T>::value || jive::IsArray<T>)
    {
        // Convert the iterable to a sparse std::map of unstructured values.
        if (structured.size() != compare.size())
        {
            // Sizes differ.
            // No compare possible.
            return Unstructure<Json>(structured);
        }

        std::map<std::string, Json> result;

        for (size_t i = 0; i < structured.size(); ++i)
        {
            auto diff = Diff<Json>(structured[i], compare[i]);

            if (diff)
            {
                result[std::to_string(i)] = *diff;
            }
        }

        if (result.empty())
        {
            return {};
        }

        return result;
    }
    else if constexpr (jive::IsBitset<T>::value)
    {
        if (structured == compare)
        {
            return {};
        }

        return structured.to_ullong();
    }
    else if constexpr (jive::IsOptional<T>)
    {
        if (!structured && !compare)
        {
            // Optionals are unset.
            // There is no diff.
            return {};
        }

        if (structured && compare)
        {
            return Diff<Json>(*structured, *compare);
        }

        return Unstructure<Json>(structured);
    }
    else if constexpr (std::is_enum_v<T>)
    {
        if (structured == compare)
        {
            return {};
        }

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
        if (structured == compare)
        {
            return {};
        }

        return structured;
    }
    else
    {
        return {};
    }
}


template<typename T, typename Json>
T & Patch(T &base, const Json &diff);

// Optionally, a class can provide it's own Patch method.
// It must be a static method that takes a single reference to a Json instance.
template<typename T, typename Json, typename = void>
struct ImplementsPatch_: std::false_type {};

template<typename T, typename Json>
struct ImplementsPatch_<
    T,
    Json,
    std::void_t<decltype(std::declval<T>().Patch(std::declval<Json>()))>
> : std::true_type {};

template<typename T, typename Json>
inline constexpr bool ImplementsPatch =
    ImplementsPatch_<T, Json>::value;


template<typename T, typename Json>
void PatchInPlace(T &base, const Json &unstructured)
{
    if constexpr (std::is_array_v<T>)
    {
        static constexpr auto size = std::extent_v<T>;

        // This is a c array
        if (unstructured.is_object())
        {
            // Apply patch only to elements that have changes.
            for (auto & [key, value]: unstructured.items())
            {
                auto index = std::stoull(key);

                if (index >= size)
                {
                    throw std::out_of_range("array index out of bounds");
                }

                PatchInPlace(base[index], value);
            }
        }
        else if (unstructured.is_array())
        {
            // Replace entire array with new values.
            assert(unstructured.size() == size);

            for (size_t i = 0; i < unstructured.size(); ++i)
            {
                PatchInPlace(base[i], unstructured[i]);
            }
        }
    }
    else if constexpr (!std::is_empty_v<T>)
    {
        Patch(base, unstructured);
    }
}


template<typename T, typename Json>
T & DoPatch(T &base, const Json &unstructured)
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
                    PatchInPlace(
                        base.*(field.member),
                        *unstructuredMember);
                }
            });
    }
    else if constexpr (!jive::IsArray<T> && CanReflect<T>)
    {
        ForEach(
            base,
            [&unstructured](const auto &name, auto &member) -> void
            {
                if (1 == unstructured.count(name))
                {
                    // Reconstruct the object from the unstructured data.
                    PatchInPlace(
                        member,
                        unstructured[name]);
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
            if (value.is_null())
            {
                // This key should be removed.
                base.erase(key);
            }
            else if (base.count(key) == 0)
            {
                // There is nothing to patch.
                // Unstructure normally.
                base[key] = Unstructure<Json>(value);
            }
            else
            {
                Patch(base[key], value);
            }
        }
    }
    else if constexpr (jive::IsValueContainer<T>::value)
    {
        if (unstructured.is_object())
        {
            // Apply patch only to elements that have changes.
            for (auto & [key, value]: unstructured.items())
            {
                auto index = std::stoull(key);
                Patch(base.at(index), value);
            }
        }
        else if (unstructured.is_array())
        {
            // Replace entire array with new values.
            base.clear();

            for (auto &value: unstructured)
            {
                base.push_back(Unstructure<Json>(value));
            }
        }
    }
    else if constexpr (jive::IsArray<T>)
    {
        // This is a std::array.
        if (unstructured.is_object())
        {
            // Apply patch only to elements that have changes.
            for (auto & [key, value]: unstructured.items())
            {
                auto index = std::stoull(key);
                Patch(base.at(index), value);
            }
        }
        else if (unstructured.is_array())
        {
            // Replace entire array with new values.
            assert(unstructured.size() == base.size());

            for (size_t i = 0; i < unstructured.size(); ++i)
            {
                Patch(base[i], unstructured[i]);
            }
        }
    }
    else if constexpr (jive::IsOptional<T>)
    {
        if (!unstructured.is_null())
        {
            if (base)
            {
                base = Patch(*base, unstructured);
            }
            else
            {
                base.emplace(unstructured);
            }
        }
    }
    else if constexpr (std::is_enum_v<T>)
    {
        if constexpr (HasToValue<T>)
        {
            base = ToValue(Tag<T>{}, std::string(unstructured));
        }
        else
        {
            base = unstructured;
        }
    }
    else
    {
        if constexpr (jive::IsString<T>::value)
        {
            base = static_cast<std::string>(unstructured);
        }
        else if constexpr (jive::IsBitset<T>::value)
        {
            base = static_cast<unsigned long long>(unstructured);
        }
        else if constexpr (!std::is_empty_v<T>)
        {
            // Other members without fields must be convertible from the json
            // value.
            base = unstructured;
        }
    }

    if constexpr (ImplementsAfterFields<T>)
    {
        // Allow T to do any additional initialization.
        base.AfterFields();
    }

    return base;
}


template<typename T, typename Json>
T & Patch(T &base, const Json &diff)
{
    if constexpr (ImplementsPatch<T, Json>)
    {
        return base.Patch(diff);
    }
    else
    {
        return DoPatch<T>(base, diff);
    }
}


} // end namespace fields
