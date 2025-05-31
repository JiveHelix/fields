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
[[nodiscard]] consteval auto GetPrettyName()
{
    return PRETTY_FUNCTION;
}

template<typename Member>
[[nodiscard]] consteval auto GetPrettyName()
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

    // sizeof("") includes the trailing null byte
    // subtract it
    static constexpr auto tail =
        pretty.substr(nameOffset + sizeof("PROBE_FIELD") - 1);
};


template<auto index, typename T>
struct MemberName_
{
    static constexpr std::string_view pretty = PrettyName<index, T>;
    static constexpr auto end = pretty.find(PrettyField::tail);
    static constexpr auto count = end - PrettyField::nameOffset;
    static constexpr auto name = pretty.substr(PrettyField::nameOffset, count);
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
