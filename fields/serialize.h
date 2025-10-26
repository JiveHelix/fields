#pragma once


#include <sstream>
#include <string>
#include <nlohmann/json.hpp>


namespace fields
{


template<typename Object>
Object FromJson(const std::string &asString)
{
    auto unstructured = nlohmann::json::parse(asString);
    return Structure<Object>(unstructured);
}


template<typename Object>
std::string ToJson(const Object &object)
{
    auto unstructured = Unstructure<nlohmann::json>(object);
    return unstructured.dump(4);
}


inline std::string FileToString(const std::string &fileName)
{
    std::ifstream input(fileName);

    if (!input)
    {
        throw std::runtime_error("Unable to open file for reading.");
    }

    std::stringstream buffer;
    buffer << input.rdbuf();

    return buffer.str();
}


} // end namespace fields
