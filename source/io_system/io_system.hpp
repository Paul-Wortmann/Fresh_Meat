#ifndef IO_SYSTEM_HPP
#define IO_SYSTEM_HPP

#include <cstdint>
#include <memory>
#include <array>
#include <GLFW/glfw3.h>
#include "io_system_defines.hpp"

class cIOSystem
{
public:
    // system base
    bool          initialize(void);
    void          terminate(void);
    void          process(void);

    // interface
    std::shared_ptr<sIO> getIOPointer(void) { return m_io; }

    // interface – raw state setters (to be called by GLFW callbacks)
    void setKey(const std::uint32_t &_key, const bool &_state) { m_io->keyMap[_key] = _state; }
    void setMousePosition(double x, double y) { m_io->mouseX = x; m_io->mouseY = y; }
    void setMouseLeftDown(bool down) { m_io->mouseLeftDown = down; }
    void setMouseRightDown(bool down) { m_io->mouseRightDown = down; }

    // interface – queries for user code
    bool getKey(const std::uint32_t &_key) const { return m_io->keyMap[_key]; }
    bool getKeyPressed(const std::uint32_t &_key) const { return m_io->keyMapReady[_key]; } // true only on the frame key became down

    double getScrollX(void) const { return m_io->scrollX; }
    double getScrollY(void) const { return m_io->scrollY; }
    double getMouseX(void) const { return m_io->mouseX; }
    double getMouseY(void) const { return m_io->mouseY; }
    bool   getMouseLeftDown(void) const { return m_io->mouseLeftDown; }
    bool   getMouseLeftPressed(void) const { return m_io->mouseLeftPressed; }
    bool   getMouseLeftReleased(void) const { return m_io->mouseLeftReleased; }
    bool   getMouseRightDown(void) const { return m_io->mouseRightDown; }
    bool   getMouseRightPressed(void) const { return m_io->mouseRightPressed; }
    bool   getMouseRightReleased(void) const { return m_io->mouseRightReleased; }

    // For resetting edge flags after processing (if needed)
    void clearEdges();

private:
    std::shared_ptr<sIO> m_io = std::make_shared<sIO>();

    // Previous frame raw states for edge detection
    std::array<bool, GLFW_KEY_LAST + 1> m_prevKeyMap = {};
    bool m_prevMouseLeftDown  = false;
    bool m_prevMouseRightDown = false;
};

#endif // IO_SYSTEM_HPP
