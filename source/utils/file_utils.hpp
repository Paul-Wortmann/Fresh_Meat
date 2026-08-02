
#ifndef UTILS_FILE_HPP
#define UTILS_FILE_HPP

#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>

// Windows includes
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif // defined

// Linux includes
#if defined(__linux__)
    #include <dirent.h>
#endif // defined

// A collection of basic functions for working with files

void        gFileToString(const std::string &_fileName, std::string &_buffer);
std::string gFileToString(const std::string &_fileName);
bool        gFileExists(const std::string &_fileName);
bool        gFileCreate(const std::string &_fileName);
bool        gDirectoryExists(const std::string &_directoryName);
bool        gDirectoryCreate(const std::string &_directoryName);
std::string gFileToBuffer(const std::string &_fileName);
bool        gFileToBufferV(const std::string &_fileName, std::vector<unsigned char> &_buffer);
std::string gFileExtension(const std::string &_fileName);
std::string gStripPath(const std::string &_fileName);

#endif // UTILS_FILE_HPP

