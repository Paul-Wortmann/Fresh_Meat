
#ifndef IO_SYSTEM_DEFINES_HPP
#define IO_SYSTEM_DEFINES_HPP

#include <array>
#include <GLFW/glfw3.h>

struct sIO
{
    // Keyboard
    std::array<bool, GLFW_KEY_LAST + 1> keyMap      = {};
    std::array<bool, GLFW_KEY_LAST + 1> keyMapReady = {};

    // Scroll
    double scrollX = 0.0f;
    double scrollY = 0.0f;

    // Mouse Pointer position
    double mouseX = 0.0f;
    double mouseY = 0.0f;

    // Mouse Left Button
    bool mouseLeftDown = false;
    bool mouseLeftPressed  = false;
    bool mouseLeftReleased = false;

    // Mouse Right Button
    bool mouseRightDown = false;
    bool mouseRightPressed = false;
    bool mouseRightReleased = false;
};

#endif // IO_SYSTEM_DEFINES_HPP

