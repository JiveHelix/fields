#pragma once


#include <jive/binary_io.h>
#include "fields/core.h"


namespace fields
{


template<typename T>
void Write(std::ostream &output, const T &value)
{
    if constexpr (HasFields<T>)
    {
        ForEachField<T>(
            [&output, &value](const auto &field) -> void
            {
                Write(output, value.*(field.member));
            });
    }
    else if constexpr (CanReflect<T>)
    {
        ForEach(
            value,
            [&output](const auto &, const auto &member)
            {
                Write(output, member);
            });
    }
    else
    {
        jive::io::Write(output, value);
    }
}


template<typename T>
T Read(std::istream &input)
{
    if constexpr (HasFields<T>)
    {
        T result;

        ForEachField<T>(
            [&input, &result](const auto &field) -> void
            {
                using Member = FieldType<decltype(field)>;
                result.*(field.member) = Read<Member>(input);
            });

        return result;
    }
    else if constexpr (CanReflect<T>)
    {
        T result{};

        ForEach(
            result,
            [&input](const auto &, auto &member)
            {
                using Member = std::remove_cvref_t<decltype(member)>;
                member = Read<Member>(input);
            });

        return result;
    }
    else
    {
        return jive::io::Read<T>(input);
    }
}


} // end namespace fields
