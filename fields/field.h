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


template<typename T>
struct IsField_: std::false_type {};


template<typename Class, typename T, typename... OtherNames>
struct IsField_<Field<Class, T, OtherNames...>>: std::true_type {};


template<typename T>
concept IsField = IsField_<T>::value;


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
struct IsHide_: std::false_type {};


template<typename Class, typename T>
struct IsHide_<Hide<Class, T>>: std::true_type {};


template<typename T>
concept IsHide = IsHide_<T>::value;


template<typename T>
struct IsHidesTuple_: std::false_type {};

template<IsHide... Hides>
struct IsHidesTuple_<std::tuple<Hides...>>: std::true_type {};

template<typename T>
concept IsHidesTuple = IsHidesTuple_<std::remove_cvref_t<T>>::value;


} // end namespace fields
