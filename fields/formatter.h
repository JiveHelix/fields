#pragma once

#include <tuple>
#include <format>
#include <string_view>
#include <fields/has_fields.h>
#include <fields/for_each.h>


namespace fields
{


template
<
    HasFields T,
    typename F = typename std::tuple_element_t<0, decltype(T::fields)>::Type
>
struct Formatter
{
    std::formatter<F> formatter;

    constexpr auto parse(std::format_parse_context & context)
    {
        return formatter.parse(context);
    }

    template<typename FormatContext>
    auto format(
        const T &hasFields,
        FormatContext &context) const
    {
        auto out = context.out();
        std::string_view separator;

        ForEachField<T>(
            [&](const auto &field) -> void
            {
                out = std::format_to(out, "{}", separator);
                out = std::format_to(out, "{}:", field.name);

                using MemberType =
                    typename std::remove_cvref_t<decltype(field)>::Type;

                if constexpr (std::same_as<MemberType, F>)
                {
                    context.advance_to(out);
                    out = formatter.format(hasFields.*(field.member), context);
                }
                else
                {
                    // Use the default formatter for this member.
                    out = std::format_to(out, "{}", hasFields.*(field.member));
                }

                // Set the separator after the first member is formatted.
                separator = ", ";
            });

        return out;
    }
};


} // end namespace fields
