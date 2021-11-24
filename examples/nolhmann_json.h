/**
  * @file nolhmann_json.h
  * 
  * @brief A wrapper around the inlucde of nlohmann/json to silence compiler
  * warnings.
  * 
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 02 Sep 2021
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

// Silence a single warning in nlohmann/json.hpp
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop
