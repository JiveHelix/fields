#pragma once


namespace fields
{


template<typename Class, typename T, typename... OtherNames>
struct Field
{
    using ClassType = Class;
    using Type = T;

    constexpr Field()
        :
        member{nullptr},
        name{nullptr},
        otherNames{}
    {

    }

    constexpr Field(
        T Class::*inMember,
        std::string_view inName,
        OtherNames ...inOtherNames)
        :
        member{inMember},
        name{inName},
        otherNames{inOtherNames...}
    {

    }

    T Class::* member;
    std::string_view name;
    std::tuple<OtherNames...> otherNames;
};


template<size_t I>
using Index = std::integral_constant<size_t, I>;


template<size_t I, typename... OtherNames>
struct IndexField
{
    static constexpr size_t index = I;

    constexpr IndexField()
        :
        otherNames{}
    {

    }

    constexpr IndexField(std::string_view inName)
        :
        name{inName},
        otherNames{}
    {

    }

    constexpr IndexField(
        Index<I>,
        std::string_view inName,
        OtherNames ...inOtherNames)
        :
        name{inName},
        otherNames{inOtherNames...}
    {

    }

    std::string_view name;
    std::tuple<OtherNames...> otherNames;
};


template<typename... OtherNames>
struct FieldNames
{
    std::string_view name;
    std::tuple<OtherNames...> otherNames;

    constexpr FieldNames(std::string_view inName, OtherNames ...inOtherNames)
        :
        name(inName),
        otherNames{inOtherNames...}
    {

    }
};

template<typename Field>
static constexpr auto CreateFieldNames(
    std::string_view reflectedName,
    const Field &field)
{
    return std::apply(
        [&](auto ...otherNames)
        {
            return FieldNames(field.name, reflectedName, otherNames...);
        },
        field.otherNames);
}


template<typename Class, typename T>
struct Hide
{
    using ClassType = Class;
    using Type = T;

    constexpr Hide()
        :
        member{nullptr}
    {

    }

    constexpr Hide(T Class::*inMember)
        :
        member{inMember}
    {

    }

    T Class::* member;
};


template<size_t I>
struct IndexHide
{
    static constexpr size_t index = I;
};


template<typename T>
struct IsAddressField_: std::false_type {};

template<typename Class, typename T, typename... OtherNames>
struct IsAddressField_<Field<Class, T, OtherNames...>>: std::true_type {};

template<typename T>
concept IsAddressField = IsAddressField_<T>::value;

template<typename T>
struct IsIndexField_: std::false_type {};

template<size_t index, typename... OtherNames>
struct IsIndexField_<IndexField<index, OtherNames...>>: std::true_type {};

template<typename T>
concept IsIndexField = IsIndexField_<T>::value;


template<typename T>
concept IsField = IsIndexField<T> || IsAddressField<T>;


template<typename T>
struct IsFieldNames_: std::false_type {};


template<typename... OtherNames>
struct IsFieldNames_<FieldNames<OtherNames...>>: std::true_type {};


template<typename T>
concept IsFieldNames = IsFieldNames_<T>::value;


template<typename T>
struct IsFieldsTuple_: std::false_type {};


template<IsField... Fields>
struct IsFieldsTuple_<std::tuple<Fields...>>: std::true_type {};


template<typename T>
concept IsFieldsTuple = IsFieldsTuple_<std::remove_cvref_t<T>>::value;


template<typename T>
struct IsAddressHide_: std::false_type {};


template<typename Class, typename T>
struct IsAddressHide_<Hide<Class, T>>: std::true_type {};


template<typename T>
concept IsAddressHide = IsAddressHide_<T>::value;


template<typename T>
struct IsIndexHide_: std::false_type {};


template<size_t I>
struct IsIndexHide_<IndexHide<I>>: std::true_type {};


template<typename T>
concept IsIndexHide = IsIndexHide_<T>::value;


template<typename T>
concept IsHide = IsAddressHide<T> || IsIndexHide<T>;


template<typename T>
struct IsHidesTuple_: std::false_type {};

template<IsHide... Hides>
struct IsHidesTuple_<std::tuple<Hides...>>: std::true_type {};

template<typename T>
concept IsHidesTuple = IsHidesTuple_<std::remove_cvref_t<T>>::value;


} // end namespace fields
