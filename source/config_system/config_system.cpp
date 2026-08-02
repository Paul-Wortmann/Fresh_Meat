
#include "config_system.hpp"

bool cConfigSystem::initialize(void)
{
    // Set initial variable state
    m_homePath   = "";
    m_configPath = "";
    m_dataPath   = "";

    #if defined(__linux__)
    // Set platform
    m_platform = ePlatform::platformLinux;

    // determine home path, try $XDG_CONFIG_HOME
    m_homePath = (std::getenv("XDG_CONFIG_HOME") != nullptr) ? std::getenv("XDG_CONFIG_HOME") : m_homePath;

    // path not found, try $HOME
    if (m_homePath.length() < 1)
        m_homePath = (std::getenv("HOME") != nullptr) ? std::getenv("HOME") : m_homePath;

    // Unable to find necessary environment variable
    if (m_homePath.length() < 1)
        return EXIT_FAILURE;

    // Define configuration full path, if not defined
    m_configPath = m_homePath + "/.config";
    if (gDirectoryExists(m_configPath) == false)
        gDirectoryCreate(m_configPath);

    m_configPath += std::string("/") + BASE_FOLDER + std::string("/");
    if (gDirectoryExists(m_configPath) == false)
        gDirectoryCreate(m_configPath);

    // Define data full path, if not defined
    m_dataPath = m_homePath + "/.local";
    if (gDirectoryExists(m_dataPath) == false)
        gDirectoryCreate(m_dataPath);

    m_dataPath += std::string("/share");
    if (gDirectoryExists(m_dataPath) == false)
        gDirectoryCreate(m_dataPath);

    m_dataPath += std::string("/") + BASE_FOLDER + std::string("/");
    if (gDirectoryExists(m_dataPath) == false)
        gDirectoryCreate(m_dataPath);

    #endif // Linux

    #if defined(_WIN32) || defined(_WIN64)
    // Set platform
    m_platform = ePlatform::platformWindows;

    // determine home path, try $APPDATA
    m_homePath = (std::getenv("APPDATA") != nullptr) ? std::getenv("APPDATA") : m_homePath;

    // path not found, try $LOCALAPPDATA
    if (m_homePath.length() < 1)
        m_homePath = (std::getenv("LOCALAPPDATA") != nullptr) ? std::getenv("LOCALAPPDATA") : m_homePath;

    // Unable to find necessary environment variable
    if (m_homePath.length() < 1)
        return EXIT_FAILURE;

    // Use / Create base folder
    m_homePath += std::string("\\") + BASE_FOLDER;
    if (gDirectoryExists(m_homePath) == false)
        gDirectoryCreate(m_homePath);

    // Define configuration full path
    m_configPath = m_homePath + "\\config\\";
    if (gDirectoryExists(m_configPath) == false)
        gDirectoryCreate(m_configPath);

    // Define data full path
    m_dataPath = m_homePath + "\\share\\";
    if (gDirectoryExists(m_dataPath) == false)
        gDirectoryCreate(m_dataPath);

    #endif // windows

    // if not exist, create
    if (gFileExists(m_configPath + CONFIG_FILE) == false)
        gFileCreate(m_configPath + CONFIG_FILE);


    if (gFileExists(m_dataPath + SAVE_FILE) == false)
        gFileCreate(m_dataPath + SAVE_FILE);


    return true;
}

bool cConfigSystem::loadConfig(const std::string &_fileName)
{
    std::string fileName = m_configPath + _fileName;
    // Load game config file
    cXML xmlFile;
    xmlFile.load(fileName);

    // Only continue if we load a file with data
    if (xmlFile.lineCount() > 0)
    {
        // Graphics
        m_resolution_x    = xmlFile.getInteger("<resolution_w>");
        m_resolution_y    = xmlFile.getInteger("<resolution_h>");
        m_fullscreen      = (xmlFile.getInteger("<fullscreen>") == 1);
        m_vsync           = (xmlFile.getInteger("<vsync>") == 1);
        m_basicRenderer   = (xmlFile.getInteger("<basic_renderer>") == 1);
        m_wireframeRender = (xmlFile.getInteger("<wireframe_render>") == 1);

        // Audio
        m_volume_master   = xmlFile.getInteger("<volume_master>");
        m_volume_music    = xmlFile.getInteger("<volume_music>");
        m_volume_sfx      = xmlFile.getInteger("<volume_sfx>");

        // Verify the data is within reasonable limits
        m_volume_master   = (m_volume_master > 100) ? 100 : m_volume_master;
        m_volume_music    = (m_volume_music > 100)  ? 100 : m_volume_music;
        m_volume_sfx      = (m_volume_sfx > 100)    ? 100 : m_volume_sfx;

        // Clean up
        xmlFile.free();
    }
    else
    {
        // Load fail
        return false;
    }

    // Load success
    return true;
}

void cConfigSystem::saveConfig(const std::string &_fileName)
{
    // Concatenate path and file name
    std::string fileName = m_configPath + _fileName;

    // Open the file, truncate, close the file.
    std::ofstream configFile;
    configFile.open(fileName, std::ofstream::out | std::ofstream::trunc);
    configFile.close();

    // Reopen to append data to the file.
    configFile.open(fileName, std::ofstream::out | std::ios_base::app);

    // Write data to the file.
    configFile << "﻿<?xml version = \"1.0\" encoding = \"UTF-8\" ?>" << std::endl;
    configFile << std::endl;
    configFile << "# This is the game configuration file." << std::endl;
    configFile << "# Delete it to have the game recreate it with safe defaults." << std::endl;
    configFile << std::endl;
    configFile << "<config>" << std::endl;
    configFile << std::endl;
    configFile << "    <graphics>" << std::endl;
    configFile << "        <resolution_w>" << m_resolution_x << "</resolution_w>" << std::endl;
    configFile << "        <resolution_h>" << m_resolution_y << "</resolution_h>" << std::endl;
    configFile << "        <vsync>" << ((m_fullscreen) ? "1" : "0") << "</vsync>" << std::endl;
    configFile << "        <fullscreen>" << ((m_fullscreen) ? "1" : "0") << "</fullscreen>" << std::endl;
    configFile << "        <basic_renderer>" << ((m_basicRenderer) ? "1" : "0") << "</basic_renderer>" << std::endl;
    configFile << "        <wireframe_render>" << ((m_wireframeRender) ? "1" : "0") << "</wireframe_render>" << std::endl;
    configFile << "    </graphics>" << std::endl;
    configFile << std::endl;
    configFile << "    <audio>" << std::endl;
    configFile << "        <volume_master>" << m_volume_master << "</volume_master>" << std::endl;
    configFile << "        <volume_music>" << m_volume_music << "</volume_music>" << std::endl;
    configFile << "        <volume_sfx>" << m_volume_sfx << "</volume_sfx>" << std::endl;
    configFile << "    </audio>" << std::endl;
    configFile << std::endl;
    configFile << "</config>" << std::endl;
    configFile << std::endl;

    // Clean up
    configFile.close();
}
