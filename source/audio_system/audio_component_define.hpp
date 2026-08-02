

#ifndef AUDIO_COMPONENT_DEFINE_HPP
#define AUDIO_COMPONENT_DEFINE_HPP

#include <string>
#include "miniaudio.hpp"

struct sComponentAudio
{
    // management
    std::string      fileName = "";
    bool             enabled  = false;

    // data
    ma_sound         data     = {};
};

#endif // AUDIO_COMPONENT_DEFINE_HPP
