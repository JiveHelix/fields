#pragma once


#include <type_traits>


namespace fields
{


static constexpr size_t maximumReflectCount = 24;


struct Probe
{

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-inline"
#endif

    // Convert to *non-aggregates* only
    template<typename T>
        requires (!std::is_aggregate_v<std::remove_cvref_t<T>>)
    constexpr operator T() const noexcept;

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

};


struct AggregateProbe
{

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-inline"
#endif

    template<typename T>
        requires(std::is_aggregate_v<std::remove_cvref_t<T>>)
    constexpr operator T() const noexcept;

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

};


template<typename T, typename... Args>
    requires(std::is_aggregate_v<std::remove_cvref_t<T>>)
consteval std::size_t ReflectMemberCount_()
{
    using Plain = std::remove_cvref_t<T>;

    if constexpr (requires { Plain{ Args{}..., AggregateProbe{} }; })
    {
        // The member at this position is an aggregate type.
        return ReflectMemberCount_<Plain, Args..., AggregateProbe>();
    }
    else if constexpr (requires { Plain{ Args{}..., Probe{} }; })
    {
        // The member at this position is not an aggregate type.
        return ReflectMemberCount_<Plain, Args..., Probe>();
    }
    else
    {
        return sizeof...(Args);
    }
}


} // end namespace fields
