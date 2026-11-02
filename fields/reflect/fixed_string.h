#pragma once


#include <cstddef>
#include <string_view>


namespace fields
{


template <size_t Length_>
struct FixedString
{
    static constexpr size_t Length = Length_;
    char data[Length]{};

    constexpr FixedString(const char (&str)[Length_])
    {
        for (size_t i = 0; i < Length; ++i)
        {
            data[i] = str[i];
        }

        data[Length - 1] = '\0';
    }

    constexpr FixedString(std::string_view view)
    {
        for (size_t i = 0; i < Length; ++i)
        {
            data[i] = view[i];
        }

        data[Length - 1] = '\0';
    }

    constexpr std::string_view view() const
    {
        // exclude null terminator
        return {&data[0], Length - 1};
    }

    constexpr std::string_view str() const
    {
        // exclude null terminator
        return {&data[0], Length - 1};
    }

    constexpr bool operator==(std::string_view rhs) const
    {
        return view() == rhs;
    }

    template<size_t M>
    constexpr bool operator==(const FixedString<M> &rhs) const
    {
        return view() == rhs.view();
    }

    constexpr const char* c_str() const { return &data[0]; }

    constexpr explicit operator const char * () const { return &data[0]; };

    template<char c>
    constexpr bool Has() const
    {
        for (size_t i = 0; i < Length; ++i)
        {
            if (data[i] == c)
            {
                return true;
            }
        }

        return false;
    }

    template<char c>
    constexpr size_t Find() const
    {
        for (size_t i = 0; i < Length; ++i)
        {
            if (data[i] == c)
            {
                return i;
            }
        }

        return Length;
    }

    friend struct FixedFriend;

private:
    constexpr FixedString()
    {
    }
};


struct FixedFriend
{
    // Make Before and After operate on template argument String.
    // This allows Find<c> to be a constant expression.
    // When SplitBefore and SplitAfter were consteval member functions of
    // FixedString, the compiler could not guarantee the contents of 'data'
    // like it can when FixedString is a template parameter.
    template<FixedString String, char c>
    struct SplitChar
    {
        static consteval auto Before()
        {
            constexpr size_t splitIndex = String.template Find<c>();

            static_assert(splitIndex != String.Length, "Character not found");

            constexpr size_t M = splitIndex + 1;
            FixedString<M> result{};

            for (size_t i = 0; i < M - 1; ++i)
            {
                result.data[i] = String.data[i];
            }

            result.data[M - 1] = '\0';

            return result;
        }

        static consteval auto After()
        {
            constexpr size_t splitIndex = String.template Find<c>();

            static_assert(splitIndex != String.Length, "Character not found");

            constexpr size_t M = String.Length - (splitIndex + 1);
            FixedString<M> result{};
            const char * start = &String.data[splitIndex + 1];

            for (size_t i = 0; i < M - 1; ++i)
            {
                result.data[i] = start[i];
            }

            result.data[M - 1] = '\0';

            return result;
        }

        static constexpr auto first = Before();
        static constexpr auto second = After();
    };

    template<FixedString check, FixedString token, size_t startIndex>
    static consteval bool CheckToken()
    {
        for (size_t i = 0; i < token.Length - 1; ++i)
        {
            if (check.data[startIndex + i] != token.data[i])
            {
                return false;
            }
        }

        return true;
    }

    template<FixedString toSearch, FixedString token>
    static consteval size_t Find()
    {
        constexpr size_t startIndex = toSearch.template Find<token.data[0]>();

        if constexpr (startIndex == toSearch.Length)
        {
            return toSearch.Length;
        }

        if constexpr (CheckToken<toSearch, token, startIndex>())
        {
            return startIndex;
        }
        else
        {
            return Find<SplitChar<toSearch, token.data[0]>::second, token>();
        }
    }

    template<FixedString Left, FixedString Middle, FixedString Right>
    static consteval auto Join()
    {
        constexpr size_t M = Left.Length + Middle.Length + Right.Length - 2;
        FixedString<M> result{};

        for (size_t i = 0; i < Left.Length - 1; ++i)
        {
            result.data[i] = Left.data[i];
        }

        for (size_t i = Left.Length - 1, j = 0; j < Middle.Length - 1; ++i, ++j)
        {
            result.data[i] = Middle.data[j];
        }

        for (
            size_t i = Left.Length + Middle.Length - 2, j = 0;
            j < Right.Length - 1;
            ++i, ++j)
        {
            result.data[i] = Right.data[j];
        }

        result.data[M - 1] = '\0';

        return result;
    }

    template<FixedString String, FixedString Token>
    struct Split
    {
        static consteval auto Before()
        {
            constexpr size_t splitIndex = Find<String, Token>();

            if constexpr (splitIndex == String.Length)
            {
                return String;
            }

            constexpr size_t M = splitIndex + 1;
            FixedString<M> result{};

            for (size_t i = 0; i < M - 1; ++i)
            {
                result.data[i] = String.data[i];
            }

            result.data[M - 1] = '\0';

            return result;
        }

        static consteval auto After()
        {
            constexpr size_t splitIndex = Find<String, Token>();

            if constexpr (splitIndex == String.Length)
            {
                return FixedString("");
            }

            constexpr size_t M = String.Length - splitIndex - Token.Length + 1;

            FixedString<M> result{};
            const char * start = &String.data[splitIndex + Token.Length - 1];

            for (size_t i = 0; i < M - 1; ++i)
            {
                result.data[i] = start[i];
            }

            result.data[M - 1] = '\0';

            return result;
        }

        static constexpr auto first = Before();
        static constexpr auto second = After();
    };
};


template<FixedString toSearch, FixedString token>
consteval size_t Find()
{
    return FixedFriend::Find<toSearch, token>();
}

static_assert(Find<"foo{}bar", "{}">() == 3);


template<FixedString String, FixedString Token>
using Split = FixedFriend::Split<String, Token>;


using TestSplit = Split<"foo{}bar", "{}">;

static_assert(TestSplit::first == "foo");
static_assert(TestSplit::second == "bar");

template<FixedString Left, FixedString Middle, FixedString Right>
consteval auto Join()
{
    return FixedFriend::Join<Left, Middle, Right>();
}



template<FixedString FormatString, FixedString First, FixedString... Others>
consteval auto Format()
{
    constexpr auto token = FixedString("{}");
    using FirstSplit = Split<FormatString, token>;
    static_assert(FirstSplit::second.Length > 0, "Format token not found");

    constexpr auto formatted =
        Join<FirstSplit::first, First, FirstSplit::second>();

    if constexpr (sizeof...(Others))
    {
        return Format<formatted, Others...>();
    }
    else
    {
        return formatted;
    }
}


static_assert(
    Format<"FieldPath<{}, {}, {}>", "foo.bar", "2", "0">()
    == "FieldPath<foo.bar, 2, 0>");


template<const std::string_view &View>
consteval auto MakeFixed()
{
    constexpr std::size_t Length = View.size() + 1;
    return FixedString<Length>(View);
}


} // end namespace fields
