#pragma once

namespace fields
{

template<typename Traits>
class EnumField
{
public:
    // using Traits = typename Traits;
    using Value = typename Traits::Value;
    static constexpr auto fieldsTypeName = Traits::fieldsTypeName;

    EnumField(): value_(Traits::defaultValue) {}
    
    explicit EnumField(const Value &value): value_(value) {}

    EnumField(const std::string &asString)
        :
        value_(Traits::valueByString.at(asString))
    {
        
    }

    EnumField(const Traits &derived)
        :
        value_(derived.value_)
    {
        
    }

    EnumField & operator=(Value value)
    {
        this->value_ = value;
        return *this;
    }

    const std::string & GetAsString() const
    {
        return Traits::stringByValue.at(this->value_);
    }

    void Set(Value value)
    {
        this->value_ = value;
    }

    Value Get() const
    {
        return this->value_;
    }

    operator Value () const { return this->value_; }

    template<typename Json>
    Json Unstructure() const
    {
        return this->GetAsString();
    }

    template<typename Json>
    static EnumField<Traits> Structure(const Json &jsonValue)
    {
        return EnumField<Traits>(jsonValue.template get<std::string>());
    }

private:
    Value value_; 
};


template<typename Traits>
std::ostream & operator<<(
    std::ostream &outputStream,
    const EnumField<Traits> &enumWrapper)
{
    return outputStream << enumWrapper.GetAsString();
}

template<typename Traits>
using StringMap = std::map<std::string, typename Traits::Value>;

template<typename Traits>
using ValueMap = std::map<typename Traits::Value, std::string>;

} // namespace fields
