#pragma once

#include <jive/zip_apply.h>


namespace fields
{


template
<
    template<typename> typename Fields,
    typename Target,
    typename Source
>
void AssignConvert(Target &target, Source &source)
{
    auto initializer = [&target, &source](
        const auto &targetField,
        const auto &sourceField) -> void
    {
        using Type = std::remove_reference_t<
            decltype(target.*(targetField.member))>;

        // Convert the source member to the Target member type prior to
        // assignment.
        target.*(targetField.member) = Type(source.*(sourceField.member));
    };

    jive::ZipApply(
        initializer,
        Fields<Target>::fields,
        Fields<Source>::fields);
}


template
<
    template<typename> typename Fields,
    typename Target,
    typename Source
>
void Assign(Target &target, Source &source)
{
    auto initializer = [&target, &source](
        const auto &targetField,
        const auto &sourceField) -> void
    {
        target.*(targetField.member) = source.*(sourceField.member);
    };

    jive::ZipApply(
        initializer,
        Fields<Target>::fields,
        Fields<Source>::fields);
}


} // end namespace fields
