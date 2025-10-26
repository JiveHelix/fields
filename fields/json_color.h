#pragma once


#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <fmt/format.h>
#include <nlohmann/json.hpp>


namespace fields
{


namespace sgr
{

    namespace ansi
    {
        inline constexpr const int bold = 1;
        inline constexpr const int underline = 4;
        inline constexpr const int reversed = 7;

        inline constexpr const int black = 30;
        inline constexpr const int red = 31;
        inline constexpr const int green = 32;
        inline constexpr const int yellow = 33;
        inline constexpr const int blue = 34;
        inline constexpr const int magenta = 35;
        inline constexpr const int cyan = 36;
        inline constexpr const int white = 37;

        namespace bright
        {
            inline constexpr const int black = 90;
            inline constexpr const int red = 91;
            inline constexpr const int green = 92;
            inline constexpr const int yellow = 93;
            inline constexpr const int blue = 94;
            inline constexpr const int magenta = 95;
            inline constexpr const int cyan = 96;
            inline constexpr const int white = 97;
        }
    }

    inline constexpr char escape = '\x1b';
    inline constexpr const char *reset = "\x1b[0m";

    inline std::string ComposeSgr(std::initializer_list<int> params)
    {
        return fmt::format("{}[{}m", escape, fmt::join(params, ";"));
    }

    inline std::string ComposeSgr(const std::vector<int> &params)
    {
        return fmt::format("{}[{}m", escape, fmt::join(params, ";"));
    }

    inline std::vector<int> ParseSgr(std::string_view asString)
    {
        std::vector<int> out;
        size_t i = 0;

        if (i < asString.size() && asString[i] == escape)
        {
            ++i;
        }

        if (i < asString.size() && asString[i] == '[')
        {
            ++i;
        }

        int code = 0;
        bool isParsingCode = false;

        for (; i < asString.size(); ++i)
        {
            char c = asString[i];

            if (std::isdigit(static_cast<unsigned char>(c)))
            {
                code = code * 10 + (c - '0');
                isParsingCode = true;
            }
            else if (c == ';')
            {
                // End of this code
                if (isParsingCode)
                {
                    out.push_back(code);
                    code = 0;
                    isParsingCode = false;
                }
            }
            else if (c == 'm')
            {
                if (isParsingCode)
                {
                    out.push_back(code);
                }

                break;
            }
        }

        return out;
    }

    inline std::string Boldize(int code)
    {
        return ComposeSgr(std::vector<int>{{1, code}});
    }

    inline std::string BoldizeSgr(std::string_view asString)
    {
        auto p = ParseSgr(asString);

        if (std::find(p.begin(), p.end(), 1) == p.end())
        {
            // Bold is not already in the string.
            p.insert(p.begin(), 1);
        }

        return ComposeSgr(p);
    }

    inline std::string Fg(int baseColor)
    {
        return ComposeSgr({baseColor});
    }

    // For symmetry with 256 adn RGB variants.
    // Identical to Fg.
    inline std::string Bg(int baseColor)
    {
        return ComposeSgr({baseColor});
    }

    inline std::string Fg256(int idx)
    {
        return ComposeSgr({38, 5, idx});
    }

    inline std::string Bg256(int idx)
    {
        return ComposeSgr({48, 5, idx});
    }

    inline std::string FgRGB(int r,int g,int b)
    {
        return ComposeSgr({38, 2, r, g, b});
    }

    inline std::string BgRGB(int r,int g,int b)
    {
        return ComposeSgr({48, 2, r, g, b});
    }
}


struct JsonFormatOptions
{
    int indentSpaces = 4;
    bool sortKeys = false;

    std::string key = sgr::Fg(sgr::ansi::bright::green);
    std::string objectKey = sgr::Fg(sgr::ansi::bright::cyan);
    std::string string = sgr::Fg(sgr::ansi::magenta);
    std::string number = sgr::Fg(sgr::ansi::bright::blue);
    std::string boolean = sgr::Fg(sgr::ansi::cyan);
    std::string nullValue = sgr::Fg(sgr::ansi::bright::black);
    std::string punctuation = sgr::Fg(sgr::ansi::yellow);

    JsonFormatOptions & IndentSpaces(int value)
    {
        this->indentSpaces = value;

        return *this;
    }

    JsonFormatOptions & SortKeys(bool value)
    {
        this->sortKeys = value;

        return *this;
    }

    JsonFormatOptions & Key(int color)
    {
        this->key = sgr::Fg(color);

        return *this;
    }

    JsonFormatOptions & ObjectKey(int color)
    {
        this->objectKey = sgr::Fg(color);

        return *this;
    }

    JsonFormatOptions & String(int color)
    {
        this->string = sgr::Fg(color);

        return *this;
    }

    JsonFormatOptions & Number(int color)
    {
        this->number = sgr::Fg(color);

        return *this;
    }

    JsonFormatOptions & Boolean(int color)
    {
        this->boolean = sgr::Fg(color);

        return *this;
    }

    JsonFormatOptions & NullValue(int color)
    {
        this->nullValue = sgr::Fg(color);

        return *this;
    }

    JsonFormatOptions & Punctuation(int color)
    {
        this->punctuation = sgr::Fg(color);

        return *this;
    }
};


class JsonAnsiPrinter
{
public:
    using json = nlohmann::ordered_json;

    JsonAnsiPrinter(
        std::ostream &os,
        const JsonFormatOptions options = {})
        :
        os_(os),
        options_(options)
    {

    }

    JsonFormatOptions & Options()
    {
        return this->options_;
    }

    void Print(const nlohmann::ordered_json &jsonElement)
    {
        this->buffer_.clear();
        this->depth_ = 0;
        this->PrintValue(jsonElement);
        fmt::format_to(std::back_inserter(this->buffer_), "\n");
        this->Flush();
    }

    void PrintRaw(std::string_view jsonText)
    {
        this->Print(nlohmann::ordered_json::parse(jsonText));
    }

private:
    std::ostream &os_;
    JsonFormatOptions options_;
    int depth_ = 0;
    fmt::memory_buffer buffer_;

    void Flush()
    {
        os_.write(
            this->buffer_.data(),
            static_cast<std::streamsize>(this->buffer_.size()));
    }

    void Indent()
    {
        const int count = this->depth_ * this->options_.indentSpaces;

        if (count > 0)
        {
            fmt::format_to(
                std::back_inserter(this->buffer_),
                "{: >{}}", "",
                count);
        }
    }

    void Punctuate(char value)
    {
        fmt::format_to(
            std::back_inserter(this->buffer_),
            "{}{}{}",
            this->options_.punctuation,
            value,
            sgr::reset);
    }

    void Append(std::string_view asString)
    {
        this->buffer_.append(
            asString.data(),
            asString.data() + asString.size());
    }

    void PrintString(
        const std::string &asString,
        const std::string &color)
    {
        // Let json do the correct quoting/escaping
        const std::string quoted = json(asString).dump();

        fmt::format_to(
            std::back_inserter(this->buffer_),
            "{}{}{}",
            color,
            quoted,
            sgr::reset);
    }

    void PrintNumber(const json &numeric)
    {
        fmt::format_to(
            std::back_inserter(this->buffer_),
            "{}{}{}",
            this->options_.number,
            numeric.dump(),
            sgr::reset);
    }

    void PrintBoolean(bool value)
    {
        fmt::format_to(
            std::back_inserter(this->buffer_),
            "{}{}{}",
            this->options_.boolean,
            (value ? "true" : "false"),
            sgr::reset);
    }

    void PrintNull()
    {
        fmt::format_to(
            std::back_inserter(this->buffer_),
            "{}null{}",
            this->options_.nullValue,
            sgr::reset);
    }

    void PrintObject(const json &obj)
    {
        this->Punctuate('{');

        if (obj.empty())
        {
            this->Punctuate('}');

            return;
        }

        this->buffer_.push_back('\n');
        ++this->depth_;

        // Stable or sorted key order
        std::vector<json::object_t::key_type> keys;

        keys.reserve(obj.size());

        for (auto it = obj.begin(); it != obj.end(); ++it)
        {
            keys.push_back(it.key());
        }

        if (this->options_.sortKeys)
        {
            std::sort(keys.begin(), keys.end());
        }

        for (size_t i = 0; i < keys.size(); ++i)
        {
            const auto &k = keys[i];
            const auto &v = obj.at(k);

            this->Indent();

            if (v.is_object())
            {
                PrintString(k, this->options_.objectKey);
            }
            else
            {
                PrintString(k, this->options_.key);
            }

            this->Punctuate(':');
            this->buffer_.push_back(' ');

            this->PrintValue(v);

            if (i + 1 < keys.size())
            {
                this->Punctuate(',');
            }

            this->buffer_.push_back('\n');
        }

        --this->depth_;
        this->Indent();
        this->Punctuate('}');
    }

    void PrintArray(const json &arr)
    {
        this->Punctuate('[');
        if (arr.empty())
        {
            this->Punctuate(']');
            return;
        }

        this->buffer_.push_back('\n');
        ++this->depth_;

        for (size_t i = 0; i < arr.size(); ++i)
        {
            this->Indent();
            this->PrintValue(arr[i]);

            if (i + 1 < arr.size())
            {
                this->Punctuate(',');
            }

            this->buffer_.push_back('\n');
        }

        --this->depth_;
        this->Indent();
        this->Punctuate(']');
    }

    void PrintValue(const json &element)
    {
        if (element.is_object())
        {
            this->PrintObject(element);
        }
        else if (element.is_array())
        {
            this->PrintArray(element);
        }
        else if (element.is_string())
        {
            this->PrintString(
                element.get_ref<const std::string&>(),
                this->options_.string);
        }
        else if (element.is_number())
        {
            this->PrintNumber(element);
        }
        else if (element.is_boolean())
        {
            this->PrintBoolean(element.get<bool>());
        }
        else if (element.is_null())
        {
            this->PrintNull();
        }
        else
        {
            // Fallback
            this->Append(element.dump());
        }
    }
};


inline std::string GetColorizedJson(
    const nlohmann::ordered_json &jsonElement,
    JsonFormatOptions options = {})
{
    std::ostringstream outputStream;
    JsonAnsiPrinter jsonPrinter(outputStream, options);
    jsonPrinter.Print(jsonElement);

    return outputStream.str();
}


} // end namespace fields
