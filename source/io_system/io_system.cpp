#include "io_system.hpp"

bool cIOSystem::initialize(void)
{
    // Clear all states
    m_io->keyMap.fill(false);
    m_io->keyMapReady.fill(false);
    m_io->mouseX = m_io->mouseY = 0.0;
    m_io->mouseLeftDown = m_io->mouseLeftPressed = m_io->mouseLeftReleased = false;
    m_io->mouseRightDown = m_io->mouseRightPressed = m_io->mouseRightReleased = false;

    m_prevKeyMap.fill(false);
    m_prevMouseLeftDown = false;
    m_prevMouseRightDown = false;

    return true;
}

void cIOSystem::terminate(void)
{
    // Nothing to clean up for this simple system
}

void cIOSystem::process(void)
{
    // 1. Keyboard edge detection (pressed only, using keyMapReady)
    for (int key = 0; key <= GLFW_KEY_LAST; ++key) {
        bool current = m_io->keyMap[key];
        bool previous = m_prevKeyMap[key];
        m_io->keyMapReady[key] = (current && !previous); // just pressed this frame
        m_prevKeyMap[key] = current;
    }

    // 2. Mouse left button edges
    bool leftCurrent = m_io->mouseLeftDown;
    m_io->mouseLeftPressed  = (leftCurrent && !m_prevMouseLeftDown);
    m_io->mouseLeftReleased = (!leftCurrent && m_prevMouseLeftDown);
    m_prevMouseLeftDown = leftCurrent;

    // 3. Mouse right button edges
    bool rightCurrent = m_io->mouseRightDown;
    m_io->mouseRightPressed  = (rightCurrent && !m_prevMouseRightDown);
    m_io->mouseRightReleased = (!rightCurrent && m_prevMouseRightDown);
    m_prevMouseRightDown = rightCurrent;
}

void cIOSystem::clearEdges()
{
    // Optionally reset edge flags if they are not consumed in the same frame
    m_io->keyMapReady.fill(false);
    m_io->mouseLeftPressed = m_io->mouseLeftReleased = false;
    m_io->mouseRightPressed = m_io->mouseRightReleased = false;
}
