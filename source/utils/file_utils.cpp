
#include "file_utils.hpp"

// A function to read data from a file and copy it to a string
// Function is called with _filename
// Returns the string in _buffer
void gFileToString(const std::string &_fileName, std::string &_buffer)
{
    std::ifstream file;
    file.open(_fileName.c_str());
    if (file.is_open())
    {
        std::stringstream strStream;
        strStream << file.rdbuf();
        _buffer = strStream.str();
        file.close();
    }
    else
    {
        std::cout << "Error - Failed to open file: " << _fileName << std::endl;
    }
}

// A function to read data from a file and return it as a string
// Function is called with _filename
// Returns a string
std::string gFileToString(const std::string &_fileName)
{
    std::ifstream fileStream(_fileName, std::ios::in);
    if (fileStream.fail())
    {
        perror(_fileName.c_str());
        std::cout << "Error - Failed to open file: " << _fileName << std::endl;
    }
    else
    {
        std::string fileData = "";
        while(!fileStream.eof())
        {
            std::string lineData = "";
            std::getline(fileStream, lineData);
            fileData.append(lineData + "\n");
        }
        fileStream.close();
        return fileData;
    }
    return nullptr;
}

// A function to check whether a file exists
// Function is called with _filename
// Returns a bool

bool gFileExists(const std::string &_fileName)
{
     std::ifstream file(_fileName.c_str());
    return file.good();
}

bool gFileCreate(const std::string &_fileName)
{
    std::ofstream file(_fileName.c_str());

    if (!file.is_open())
        return false;

    file.close();

    return true;
}

bool gDirectoryExists(const std::string &_directoryName)
{
    struct stat info;

    if (stat(_directoryName.c_str(), &info) != 0)
        return false;

    if (info.st_mode & S_IFDIR)
        return true;

    return false;
}

bool gDirectoryCreate(const std::string &_directoryName)
{
    #if defined(_WIN32) || defined(_WIN64)
        return (CreateDirectory(_directoryName.c_str(), NULL));
    #endif // windows

    #if defined(__linux__)
        return (mkdir(_directoryName.c_str(), 0777) == 0);
    #endif // __linux__

    return false;
}

std::string gFileToBuffer(const std::string &_fileName)
{
    std::ifstream inFile(_fileName);
    if (inFile.good())
    {
        inFile.seekg(0, std::ios::end);
        size_t size = inFile.tellg();
        std::string buffer(size, ' ');
        inFile.seekg(0);
        inFile.read(&buffer[0], size);
        inFile.close();
        return buffer;
    }
    return nullptr;
}

// A function to read data from a file and copy it to a unsigned char vector
// Function is called with _filename
// Returns the data in _buffer
// Returns a bool to indicate success or failure
bool gFileToBufferV(const std::string &_fileName, std::vector<unsigned char> &_buffer)
{
    std::fstream fileStream(_fileName.c_str(), std::ios::in | std::ios::binary);
    if (fileStream.fail())
    {
        perror(_fileName.c_str());
        std::cout << "Error - Failed to open file: " << _fileName << std::endl;
        return false;
    }
    fileStream.seekg(0, std::ios::end);
    std::uint64_t file_size = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);
    file_size -= fileStream.tellg();
    _buffer.resize(file_size);
    fileStream.read((char*)&(_buffer[0]), file_size);
    fileStream.close();
    return true;
}

// A function to return a file's extension
// Function is called with _filename
// Returns the data as a string
std::string gFileExtension(const std::string &_fileName)
{
    const char16_t marker = '.';
    std::string r_returnString = "";
    bool markerFound = false;
    std::uint64_t fileLength = _fileName.length();
    for (std::int64_t i = fileLength-1; i >= 0; i--)
    {
        if (!markerFound)
        {
            if (_fileName[i] != marker)
                r_returnString += _fileName[i];
            else
                markerFound = true;
        }
    }
    std::uint64_t r_fileLength = r_returnString.length();
    std::string returnString = "";
    for (std::int64_t i = r_fileLength-1; i >= 0; i--)
    {
            returnString += r_returnString[i];
    }
    return returnString;
}

// A function to remove a files path and return only the file's name and extension
// Function is called with _filename
// Returns the data as a string
std::string gStripPath(const std::string &_fileName)
{
    const char16_t marker = '/';
    std::string r_returnString = "";
    bool markerFound = false;
    std::uint64_t fileLength = _fileName.length();
    for (std::int64_t i = fileLength-1; i >= 0; i--)
    {
        if (!markerFound)
        {
            if (_fileName[i] != marker)
                r_returnString += _fileName[i];
            else
                markerFound = true;
        }
    }
    std::uint64_t r_fileLength = r_returnString.length();
    std::string returnString = "";
    for (std::int64_t i = r_fileLength-1; i >= 0; i--)
    {
            returnString += r_returnString[i];
    }
    return returnString;
}
