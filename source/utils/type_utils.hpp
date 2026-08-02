
#ifndef TYPE_UTILS_HPP
#define TYPE_UTILS_HPP

#include <cstdint>
#include <string>
#include <glm/glm.hpp>

// A collection of basic functions for working with data types

void gStringToInt32Array(const std::string &_string, const std::uint32_t &_dataCount, std::uint32_t *&_array);
void gStringToFloatArray(const std::string &_string, const std::uint32_t &_dataCount, float *&_array);
void gStringToMat4Array(const std::string &_string, const std::uint32_t &_dataCount, glm::mat4 *&_array);

#endif // TYPE_UTILS_HPP

