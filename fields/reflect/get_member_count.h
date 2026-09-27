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
    template<class T>
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

    template<class T>
        requires(std::is_aggregate_v<std::remove_cvref_t<T>>)
    constexpr operator T() const noexcept;

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

};


template<class T, class... Args>
    requires(std::is_aggregate_v<std::remove_cvref_t<T>>)
consteval std::size_t GetMemberCount()
{
    using Plain = std::remove_cvref_t<T>;

    if constexpr (requires { Plain{ Args{}..., AggregateProbe{} }; })
    {
        // The member at this position is an aggregate type.
        return GetMemberCount<Plain, Args..., AggregateProbe>();
    }
    else if constexpr (requires { Plain{ Args{}..., Probe{} }; })
    {
        // The member at this position is not an aggregate type.
        return GetMemberCount<Plain, Args..., Probe>();
    }
    else
    {
        return sizeof...(Args);
    }
}


} // end namespace fields
