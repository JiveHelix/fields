#pragma once

#include <jive/zip_apply.h>
#include <fields/has_fields.h>


namespace fields
{


template
<
    typename Target,
    typename Source
>
void AssignConvert(Target &target, Source &source)
{
    if constexpr (HasFields<Target> && HasFields<Source>)
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

        jive::ZipApply(initializer, Target::fields, Source::fields);
    }
    else
    {
        auto initializer = [](
            auto &targetMember,
            const auto &sourceMember) -> void
        {
            using Type = std::remove_reference_t<decltype(targetMember)>;

            // Convert the source member to the Target member type prior to
            // assignment.
            targetMember = Type(sourceMember);
        };

        ForEachZip(target, source, initializer);
    }
}


template
<
    typename Target,
    typename Source
>
void Assign(Target &target, const Source &source)
{
    if constexpr (HasFields<Target> && HasFields<Source>)
    {
        auto initializer = [&target, &source](
            const auto &targetField,
            const auto &sourceField) -> void
        {
            target.*(targetField.member) = source.*(sourceField.member);
        };

        jive::ZipApply(
            initializer,
            Target::fields,
            Source::fields);
    }
    else
    {
        auto initializer = [](
            auto &targetMember,
            const auto &sourceMember) -> void
        {
            targetMember = sourceMember;
        };

        ForEachZip(target, source, initializer);
    }
}


template
<
    typename Target,
    typename Source
>
void MoveAssign(Target &target, Source &&source)
{
    if constexpr (HasFields<Target> && HasFields<Source>)
    {
        auto initializer = [&target, &source](
            const auto &targetField,
            const auto &sourceField) -> void
        {
            target.*(targetField.member) =
                std::move(source.*(sourceField.member));
        };

        jive::ZipApply(
            initializer,
            Target::fields,
            Source::fields);
    }
    else
    {
        auto initializer = [](
            auto &targetMember,
            auto &&sourceMember) -> void
        {
            targetMember = std::move(sourceMember);
        };

        ForEachZip(target, std::forward<Source>(source), initializer);
    }
}


} // end namespace fields
