#pragma once


#include <string_view>

#include "get_member_tuple.h"
#include "get_member_count.h"


namespace fields
{


#if defined(__clang__) || defined(__GNUC__)
#   define PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined (_MSC_VER)
#   define PRETTY_FUNCTION __FUNCSIG__
#else
#   error Unsupported compiler
#endif



template<auto Member>
[[nodiscard]] consteval std::string_view GetPrettyName()
{
    return PRETTY_FUNCTION;
}

template<typename Member>
[[nodiscard]] consteval std::string_view GetPrettyName()
{
    return PRETTY_FUNCTION;
}


template <class T>
struct Pointer
{
    const T *value;
};


template<size_t index, class T>
constexpr auto GetPointer(T&& t)
{
    auto &member = std::get<index>(GetMemberTuple(t));
    return Pointer<std::remove_cvref_t<decltype(member)>>{&member};
}


template <class T>
extern const T inspect;


template<size_t I, typename T>
struct MemberType_
{
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
    using Members = decltype(GetMemberTuple(inspect<T>));
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    using Type = std::remove_cvref_t<std::tuple_element_t<I, Members>>;
};

template<size_t I, typename T>
using MemberType = typename MemberType_<I, T>::Type;


template<auto N, class T>
inline constexpr auto PrettyName =
    []() constexpr
    {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
        return GetPrettyName<GetPointer<N>(inspect<T>)>();
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    }();


struct ProbePretty
{
    int PROBE_FIELD;
};


struct PrettyField
{
    static constexpr std::string_view pretty = PrettyName<0, ProbePretty>;
    static constexpr auto nameOffset = pretty.find("PROBE_FIELD");

    static_assert(
        nameOffset < pretty.size(),
        "Find returned past end - possible dangling string_view");

    // sizeof("") includes the trailing null byte
    // subtract it
    static constexpr auto tail =
        pretty.substr(nameOffset + sizeof("PROBE_FIELD") - 1);

#if __GNUC__
    // pretty includes the name of the containing structure, twice.
    static constexpr auto extraStructNameOffset =
        2 * (sizeof("PrettyField") - 1);

    // It also includes the name of the type, with inconsistent spacing inside
    // Pointer<...>.
    // Pointer<int> is fine, but Pointer<std::vector<int> > adds an extra
    // space.
    // Hopefully all of this brittleness goes away with C++26.
    static constexpr auto extraTypeNameOffset =
        jive::detail::TypeName<Pointer<int>>().size();
#endif
};


template<auto index, typename T>
struct MemberName_
{
    static constexpr std::string_view pretty = PrettyName<index, T>;
    static constexpr auto end = pretty.find(PrettyField::tail);

#if __GNUC__
    static constexpr auto nameOffset =
        PrettyField::nameOffset
        - PrettyField::extraStructNameOffset
        + (2 * jive::detail::TypeName<T>().size())
        - PrettyField::extraTypeNameOffset
        + jive::detail::TypeName<Pointer<MemberType<index, T>>>().size();
#else
    static constexpr auto nameOffset = PrettyField::nameOffset;
#endif

    static constexpr auto count = end - nameOffset;
    static_assert(nameOffset < pretty.size());
    static_assert(nameOffset + count <= pretty.size());
    static constexpr auto name = pretty.substr(nameOffset, count);
};


template<auto index, typename T>
inline constexpr auto MemberName = MemberName_<index, T>::name;


template<typename T>
struct MemberNames_
{
    using Type = std::remove_cvref_t<T>;

    static constexpr auto names =
        []<size_t... Is>(std::index_sequence<Is...>)
        {
            return std::array{MemberName<Is, Type>...};
        }
        (std::make_index_sequence<GetMemberCount<Type>()>{});
};


template<typename T>
inline constexpr auto MemberNames = MemberNames_<T>::names;


} // end namespace fields
