

#include <cstdint>
#include <iostream>

#include "audio_system.hpp"

void cAudioSystem::freeAllAudio(void)
{
    // free music
    if (m_music.pDataSource)
    {
        ma_sound_uninit(&m_music);
    }

    // free all sounds in the forward_list
    for (auto& comp : m_components)
    {
        if (comp.data.pDataSource != nullptr)
            ma_sound_uninit(&comp.data);
        comp.enabled = false;
    }

    // clear containers
    m_components.clear();
    m_componentIndex.clear();
    m_freeList.clear();
    m_nextIndex = 0;
}

std::uint32_t cAudioSystem::m_getNewComponent(void)
{
    // reuse a freed component if available
    if (!m_freeList.empty())
    {
        std::uint32_t index = m_freeList.back();
        m_freeList.pop_back();

        // locate the component via map and reset it
        auto it = m_componentIndex.find(index);
        if (it != m_componentIndex.end() && it->second != nullptr)
        {
            sComponentAudio* comp = it->second;
            *comp = sComponentAudio{};      // reset to default state
            comp->enabled = true;
            return index;
        }
        else
        {
            // Should never happen – fallback to creating a new component
            std::cout << "Warning: free index " << index << " not found in map, creating new." << std::endl;
        }
    }

    // create a brand new component
    std::uint32_t newIndex = m_nextIndex++;
    m_components.emplace_front();
    sComponentAudio* newComp = &m_components.front();
    newComp->enabled = true;
    m_componentIndex[newIndex] = newComp;
    return newIndex;
}

void cAudioSystem::freeComponent(const std::uint32_t &_index)
{
    // check existence in map
    auto it = m_componentIndex.find(_index);
    if (it == m_componentIndex.end())
        return;

    sComponentAudio* comp = it->second;
    if (!comp || !comp->enabled)
        return;

    // free miniaudio data
    if (comp->data.pDataSource != nullptr)
    {
        ma_sound_uninit(&comp->data);
        comp->data = {};
    }

    comp->enabled = false;
    m_freeList.push_back(_index);
}

bool cAudioSystem::initialize(void)
{
    // Start audio engine
    m_result = ma_engine_init(NULL, &m_engine);
    if (m_result != MA_SUCCESS)
    {
        std::cout << "Failed to start audio engine: " + std::string(ma_result_description(m_result)) << std::endl;
        return false;
    }

    // version
    std::cout << "Miniaudio version: " + std::string(MA_VERSION_STRING) << std::endl;

    return true;
}

void cAudioSystem::terminate(void)
{
    // free all audio
    freeAllAudio();

    // terminate engine
    ma_engine_uninit(&m_engine);
}

void cAudioSystem::process(float _delta)
{

}

std::uint32_t cAudioSystem::loadSound(const std::string &_fileName)
{
    // first see if the sound is already loaded (enabled and same filename)
    for (auto& comp : m_components)
    {
        if (comp.enabled && comp.fileName == _fileName)
        {
            // find its index by scanning the map (inefficient but simple)
            for (const auto& pair : m_componentIndex)
            {
                if (pair.second == &comp)
                    return pair.first;
            }
        }
    }

    // allocate a new component slot
    std::uint32_t index = m_getNewComponent();
    sComponentAudio* comp = m_componentIndex[index];
    comp->fileName = _fileName;

    m_result = ma_sound_init_from_file(&m_engine, _fileName.c_str(), 0, NULL, NULL, &comp->data);
    if (m_result != MA_SUCCESS)
    {
        std::cout << "Failed to load sound: " + _fileName + " - " + ma_result_description(m_result) << std::endl;
        // mark as disabled so it can be reused later
        freeComponent(index);
        return 0;   // invalid index
    }
    else
    {
        ma_sound_set_volume(&comp->data, static_cast<float>(m_volumeSound) / static_cast<float>(m_volumeMax));
    }

    return index;
}

void cAudioSystem::playSound(const std::uint32_t &_index)
{
    auto it = m_componentIndex.find(_index);
    if (it == m_componentIndex.end())
    {
        std::cout << "Invalid sound index." << std::endl;
        return;
    }

    sComponentAudio* comp = it->second;
    if (!comp || !comp->enabled)
    {
        std::cout << "Sound not loaded or already freed." << std::endl;
        return;
    }

    m_result = ma_sound_start(&comp->data);
    if (m_result != MA_SUCCESS)
    {
        std::cout << "Failed to play sound: " + std::string(ma_result_description(m_result)) << std::endl;
    }
}

void cAudioSystem::loadMusic(const std::string &_fileName)
{
    // if data, free first
    if (m_music.pDataSource)
    {
        ma_sound_stop(&m_music);
        ma_sound_uninit(&m_music);
    }

    m_result = ma_sound_init_from_file(&m_engine, _fileName.c_str(), 0, NULL, NULL, &m_music);
    if (m_result != MA_SUCCESS)
    {
        std::cout << "Failed to load music: " + std::string(ma_result_description(m_result)) << std::endl;
    }
    else
    {
        ma_sound_set_volume(&m_music, static_cast<float>(m_volumeMusic) / static_cast<float>(m_volumeMax));
        ma_sound_set_looping(&m_music, true);
    }
}

void cAudioSystem::playMusic(void)
{
    m_result = ma_sound_start(&m_music);
    if (m_result != MA_SUCCESS)
    {
        std::cout << "Failed to play music: " + std::string(ma_result_description(m_result)) << std::endl;
    }
}

void cAudioSystem::m_updateVolume(void)
{
    // Master volume
    ma_device* device = ma_engine_get_device(&m_engine);
    m_result = ma_device_set_master_volume(device, static_cast<float>(m_volumeMaster) / static_cast<float>(m_volumeMax));
    if (m_result != MA_SUCCESS)
    {
        std::cout << "Failed to set master volume : " + std::string(ma_result_description(m_result)) << std::endl;
    }

    float musicVolume = static_cast<float>(m_volumeMusic) / static_cast<float>(m_volumeMax);
    float soundVolume = static_cast<float>(m_volumeSound) / static_cast<float>(m_volumeMax);

    // music volume
    if (m_music.pDataSource != nullptr)
    {
        ma_sound_set_volume(&m_music, musicVolume);
    }

    // sound volumes – iterate over all enabled components
    for (auto& comp : m_components)
    {
        if (comp.enabled && comp.data.pDataSource != nullptr)
            ma_sound_set_volume(&comp.data, soundVolume);
    }
}
