#pragma once


#include <type_traits>


namespace fields
{


struct TestType
{
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-inline"
#endif
    template<class T>
    constexpr operator T() const noexcept;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
};


template<typename T, typename... Args>
    requires (std::is_aggregate_v<std::remove_cvref_t<T>>)
consteval size_t GetMemberCount()
{
    using Plain = std::remove_cvref_t<T>;

    if constexpr (requires { Plain{Args{}..., TestType{}}; })
    {
        // Our type T can be constructed with the existing Args plus one more.
        // Keep going.
        return GetMemberCount<Plain, Args..., TestType>();
    }
    else if constexpr (requires { Plain{{Args{}..., TestType{}}}; })
    {
        return GetMemberCount<Plain, Args..., TestType>();
    }
    else
    {
        // Our type would not accept any more initializers.
        return sizeof...(Args);
    }
}


#include <optional>

struct OptionalTest
{
    int x;
    double y;
    std::optional<short> z;
};

static_assert(GetMemberCount<OptionalTest>() == 3);


struct EmptyTest
{

};


static_assert(GetMemberCount<EmptyTest>() == 0);


} // end namespace fields
