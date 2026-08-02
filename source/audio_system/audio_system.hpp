#ifndef AUDIO_SYSTEM_HPP
#define AUDIO_SYSTEM_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <forward_list>
#include <map>

#include "miniaudio.hpp"
#include "audio_component_define.hpp"
#include "audio_event_define.hpp"
#include "../template/template_queue.hpp"

class cAudioSystem
{
public:
    // base interface
    bool initialize(void);
    void terminate(void);
    void process(float _delta);

    // event interface
    sAudioEvent* getEvent(void) { return m_event.pop(); }

    // component interface
    void freeComponent(const std::uint32_t &_index);

    // sound interface
    std::uint32_t    loadSound(const std::string &_fileName);
    void             playSound(const std::uint32_t &_index);
    void             loadMusic(const std::string &_fileName);
    void             playMusic(void);
    void             freeAllAudio(void);

    // Volume interface
    void             setVolumeMaster(std::uint32_t _volume) {m_volumeMaster = _volume; m_updateVolume(); };
    std::uint32_t    getVolumeMaster(void) {return m_volumeMaster; };
    void             setVolumeMasterUp(void) { if (m_volumeMaster < m_volumeMax) m_volumeMaster++; m_updateVolume(); };
    void             setVolumeMasterDown(void) { if (m_volumeMaster > 0) m_volumeMaster--; m_updateVolume(); };
    void             setVolumeMusic(std::uint32_t _volume) {m_volumeMusic = _volume; m_updateVolume(); };
    std::uint32_t    getVolumeMusic(void) {return m_volumeMusic; };
    void             setVolumeMusicUp(void) { if (m_volumeMusic < m_volumeMax) m_volumeMusic++; m_updateVolume(); };
    void             setVolumeMusicDown(void) { if (m_volumeMusic > 0) m_volumeMusic--; m_updateVolume(); };
    void             setVolumeSound(std::uint32_t _volume) {m_volumeSound = _volume; m_updateVolume(); };
    std::uint32_t    getVolumeSound(void) {return m_volumeSound; };
    void             setVolumeSoundUp(void) { if (m_volumeSound < m_volumeMax) m_volumeSound++; m_updateVolume(); };
    void             setVolumeSoundDown(void) { if (m_volumeSound > 0) m_volumeSound--; m_updateVolume(); };

private:
    // component storage
    std::forward_list<sComponentAudio>           m_components;
    std::map<std::uint32_t, sComponentAudio*>    m_componentIndex;
    std::vector<std::uint32_t>                   m_freeList;
    std::uint32_t                                m_nextIndex = 0;

    std::uint32_t m_getNewComponent(void);

    // Event
    tcQueue<sAudioEvent> m_event = {};

    // miniaudio engine
    ma_result            m_result = {};
    ma_engine            m_engine = {};

    // volume
    void                 m_updateVolume(void);
    const std::uint32_t  m_volumeMax     = 100;
    std::uint32_t        m_volumeMaster  = 100;
    std::uint32_t        m_volumeMusic   = 100;
    std::uint32_t        m_volumeSound   = 100;

    // music
    ma_sound             m_music = {};
};

#endif // AUDIO_SYSTEM_HPP
