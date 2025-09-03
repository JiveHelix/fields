#pragma once


#include <type_traits>


namespace fields
{


static constexpr size_t maximumReflectCount = 16;


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
consteval std::size_t GetBaseMemberCount()
{
    using Plain = std::remove_cvref_t<T>;

    if constexpr (requires { Plain{{ Args{}..., AggregateProbe{} }}; })
    {
        return GetBaseMemberCount<Plain, Args..., AggregateProbe>();
    }
    else if constexpr (requires { Plain{{ Args{}..., Probe{} }}; })
    {
        return GetBaseMemberCount<Plain, Args..., Probe>();
    }
    else
    {
        return sizeof...(Args);
    }
}


template<class T, class... Args>
    requires(std::is_aggregate_v<std::remove_cvref_t<T>>)
consteval std::size_t GetMemberCount()
{
    using Plain = std::remove_cvref_t<T>;

    if constexpr (requires { Plain{ Args{}..., AggregateProbe{} }; })
    {
        return GetMemberCount<Plain, Args..., AggregateProbe>();
    }
    else if constexpr (requires { Plain{ Args{}..., Probe{} }; })
    {
        return GetMemberCount<Plain, Args..., Probe>();
    }
    else if constexpr (sizeof...(Args) == 1)
    {
        if constexpr (requires { Plain{ AggregateProbe{} }; })
        {
            // Open up the base class
            return GetBaseMemberCount<Plain>();
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return sizeof...(Args);
    }
}


} // end namespace fields
