
#ifndef XML_UTILS_HPP
#define XML_UTILS_HPP

#include <string>
#include <fstream>
#include <cstdint>
#include <streambuf>
#include <string>

std::string gRemoveTagsFromString(const std::string &_string);
std::string gRemoveSpacesFromString(const std::string &_string);
std::string gExtractValueFromCsvString(const std::string &_string, std::uint32_t _position);

#endif //XML_UTILS_HPP

