#pragma once

#include <tuple>
#include <fmt/format.h>
#include <string_view>
#include <fields/has_fields.h>
#include <fields/for_each.h>
#include <fields/core.h>


namespace fields
{


template<CanInspect T, typename = void>
struct FirstFieldType_ {};


template<CanInspect T>
struct FirstFieldType_
<
    T,
    std::enable_if_t<HasFields<T>>
>
{
    using Type = typename std::tuple_element_t<0, decltype(T::fields)>::Type;
};


template<CanInspect T>
struct FirstFieldType_
<
    T,
    std::enable_if_t<CanReflect<T>>
>
{
    using Type = Reflect<T>::template Element<0>;
};


template<CanInspect T>
using FirstFieldType = typename FirstFieldType_<T>::Type;


template
<
    CanInspect T,
    typename F = FirstFieldType<T>
>
struct Formatter
{
    fmt::formatter<F> formatter;

    constexpr auto parse(fmt::format_parse_context & context)
    {
        return formatter.parse(context);
    }

    template<typename FormatContext>
    auto format(
        const T &canInspect,
        FormatContext &context) const
    {
        auto out = context.out();
        std::string_view separator;

        if constexpr (HasFields<T>)
        {
            ForEachField<T>(
                [&](const auto &field) -> void
                {
                    out = fmt::format_to(out, "{}", separator);
                    out = fmt::format_to(out, "{}:", field.name);

                    using MemberType =
                        typename std::remove_cvref_t<decltype(field)>::Type;

                    if constexpr (std::same_as<MemberType, F>)
                    {
                        context.advance_to(out);

                        out = formatter.format(
                            canInspect.*(field.member),
                            context);
                    }
                    else
                    {
                        // Use the default formatter for this member.
                        out = fmt::format_to(
                            out,
                            "{}",
                            canInspect.*(field.member));
                    }

                    // Set the separator after the first member is formatted.
                    separator = ", ";
                });
        }
        else
        {
            static_assert(
                CanReflect<T>,
                "CanInspect tests HasFields or CanReflect");

            ForEach(
                canInspect,
                [&](const auto &fieldName, const auto &member) -> void
                {
                    out = fmt::format_to(out, "{}", separator);
                    out = fmt::format_to(out, "{}:", fieldName.name);

                    using MemberType =
                        typename std::remove_cvref_t<decltype(member)>;

                    if constexpr (std::same_as<MemberType, F>)
                    {
                        // This member has the type of the existing formatter.
                        context.advance_to(out);
                        out = formatter.format(member, context);
                    }
                    else
                    {
                        // Use the default formatter for this member.
                        out = fmt::format_to(out, "{}", member);
                    }

                    // Set the separator after the first member is formatted.
                    separator = ", ";
                });

        }

        return out;
    }
};


} // end namespace fields
