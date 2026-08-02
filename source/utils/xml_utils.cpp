
#include "xml_utils.hpp"

// Extract string data from between <string>data</string>
std::string gRemoveTagsFromString(const std::string &_string)
{
    std::string startDelimiter = ">";
    std::string endDelimiter = "<";

    size_t startPos = _string.find(startDelimiter);
    startPos += startDelimiter.length();

    size_t endPos = _string.find(endDelimiter, startPos);
    size_t length = endPos - startPos;
    return _string.substr(startPos, length);
}

// Remove spaces from string
std::string gRemoveSpacesFromString(const std::string &_string)
{
    std::uint32_t stringLength = _string.length();
    std::string tempString = "";

    for (std::uint32_t i = 0; i < stringLength; ++i)
    {
        if (_string[i] != ' ')
            tempString += _string[i];
    }

    return tempString;
}

// Extract Nth value from a CSV from string
std::string gExtractValueFromCsvString(const std::string &_string, std::uint32_t _position)
{
    std::string dataString = std::string(_string + std::string(","));
    std::string tempString = "";
    std::uint32_t commaCount = 0;
    std::uint32_t stringLength = dataString.length();

    for (std::uint32_t i = 0; i < stringLength; ++i)
    {
        if (dataString[i] == ',')
            commaCount++;

        if ((commaCount == _position) && (dataString[i] != ','))
            tempString += dataString[i];
    }

    return tempString;
}
