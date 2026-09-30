#pragma once


#include <jive/describe_type.h>
#include <jive/optional.h>

#include <fields/has_fields.h>
#include <fields/reflect/get_member_count.h>
#include <fields/reflect/member_names.h>
#include <fields/reflect/reflect_traits.h>


namespace fields
{


template<typename T>
    requires CanReflect<T>
constexpr auto && ForwardReflector(T &&t)
{
    using Type = ReflectorType<T>;

    using Base = std::conditional_t
        <
            std::is_const_v<std::remove_reference_t<T>>,
            const Type,
            Type
        >;

    using Reference = std::conditional_t
        <
            std::is_lvalue_reference_v<T>,
            Base &,
            Base &&
        >;

    return static_cast<Reference>(t);
}


template<CanReflect T>
struct Reflect
{
    using Type = ReflectorType<T>;

    static constexpr auto count = GetMemberCount<Type>();

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
    using Members = decltype(GetMemberTuple(inspect<Type>));
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    static constexpr auto names = MemberNames<Type>;

    template<size_t I>
    using Element = std::remove_cvref_t<std::tuple_element_t<I, Members>>;

    template<size_t I>
    static constexpr auto name = std::get<I>(names);
};



template<CanReflect T>
void PrintMemberTypes(std::ostream &output)
{
    using Reflection = Reflect<T>;

    [&output]<size_t... Is>(std::index_sequence<Is...>)
    {
        ((output << "name: " << std::get<Is>(Reflection::names)
            << ", type: "
            << jive::GetTypeName<typename Reflection::template Element<Is>>()
            << '\n'), ...);
    }(std::make_index_sequence<Reflection::count>{});
}


} // end namespace fields
