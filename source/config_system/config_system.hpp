
#ifndef CONFIG_SYSTEM_HPP
#define CONFIG_SYSTEM_HPP

#include "../core/defines.hpp"
#include "../utils/file_utils.hpp"
#include "../utils/xml_parser.hpp"

enum class ePlatform: std::uint32_t
{
    platformUnsupported = 0,
    platformLinux       = 1,
    platformWindows     = 2
};

class cConfigSystem
{
    public:
       bool initialize(void);
       bool loadConfig(const std::string &_fileName = CONFIG_FILE);
       void saveConfig(const std::string &_fileName = CONFIG_FILE);

       // system paths
       std::string getConfigPath(void) { return m_configPath; }
       std::string getDataPath(void)   { return m_dataPath; }
       std::string getHomePath(void)   { return m_homePath; }

       // configuration data
        std::uint32_t getVolumeMaster(void) { return m_volume_master; }
        std::uint32_t getVolumeMusic(void) { return m_volume_music; }
        std::uint32_t getVolumeSound(void) { return m_volume_sfx; }
        std::uint32_t getResolutionX(void) { return m_resolution_x; }
        std::uint32_t getResolutionY(void) { return m_resolution_y; }
        bool          getVsync(void) { return m_vsync; }
        bool          getFullscreen(void) { return m_fullscreen; }

    protected:

    private:
        // The platform and paths are determined on initialization
        ePlatform   m_platform   = ePlatform::platformUnsupported;
        std::string m_configPath = {};
        std::string m_dataPath   = {};
        std::string m_homePath   = {};

        // These should all be set to low default values
        // The graphics engine will use the display's native resolution when fullscreen,
        // and will use the closest valid resolution to these values when not.

        // Graphics
        std::uint32_t m_resolution_x    = 1920;
        std::uint32_t m_resolution_y    = 1080;
        bool          m_vsync           = true;
        bool          m_fullscreen      = true;
        bool          m_basicRenderer   = false;
        bool          m_wireframeRender = false;

        // Audio
        std::uint32_t m_volume_max      = 100;
        std::uint32_t m_volume_master   = 50;
        std::uint32_t m_volume_music    = 50;
        std::uint32_t m_volume_sfx      = 50;
};

#endif // CONFIG_SYSTEM_HPP

