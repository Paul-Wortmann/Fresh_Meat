// ============================================================================
// ui_system.hpp
// ============================================================================

#ifndef UI_SYSTEM_HPP
#define UI_SYSTEM_HPP

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <queue>

#include "../template/template_queue.hpp" // for tcQueue<T>
#include "../audio_system/audio_system.hpp"
#include "../graphics_system/graphics_system.hpp"
#include "../io_system/io_system_defines.hpp"
#include "ui_define.hpp"
#include "ui_event_define.hpp"

class cUISystem
{
public:
    // Base functions
    bool initialize(void);
    void terminate(void);
    bool process(float _delta);

    // External interface
    void setIOPointer(std::shared_ptr<sIO> _io) { m_io = _io; }
    void setGraphicsSystem(cGraphicsSystem* _graphicsSystem) { m_graphicsSystem = _graphicsSystem; }
    void setAudioSystem(cAudioSystem* _audioSystem) { m_audioSystem = _audioSystem; }

    // System Interface
    sUIEvent* getEvent(void) { return m_event.pop(); }
    bool loadUI(const std::string& _fileName);
    void setWindowDimensions(const std::uint32_t& _width, const std::uint32_t& _height) { m_windowWidth = _width; m_windowHeight = _height; }

    // Dynamic UI updates
    void setFormEnabled(const int32_t& _formID, const bool& _state);
    void setProgressBar(const int32_t& _formID, const int32_t& _elementID, const float& _value);
    void setSliderValue(const int32_t& _formID, const int32_t& _elementID, const float& _value);
    void setDropdownSelection(const int32_t& _formID, const int32_t& _elementID, const int32_t& _index);
    void setButtonText(const int32_t& _formID, const int32_t& _elementID, const std::string& _text);
    void setTitleBarText(const int32_t& _formID, const int32_t& _elementID, const std::string& _text);
    int32_t getFormIndexByName(const std::string& _name) const;
    bool getFormEnabled(const int32_t& _formID);

    // Global UI access
    sUIColor& getUIColor() { return m_uiColor; }
    std::vector<sUIForm>& getUIForms(void) { return m_forms; }

private:
    // member variables
    std::vector<sUIForm>  m_forms;
    std::vector<uint32_t> m_freeList;
    sUIColor              m_uiColor  = {};
    sUIAudio              m_uiAudio  = {};

    // event
    tcQueue<sUIEvent> m_event;

    // External pointers
    std::shared_ptr<sIO> m_io;
    cGraphicsSystem* m_graphicsSystem = nullptr;
    cAudioSystem* m_audioSystem = nullptr;

    uint32_t getNewForm(void);
    void destroyForm(const uint32_t& _index);
    bool isMouseInside(const glm::vec2& _pos, const glm::vec2& _dimensions, const glm::vec2& _mousePos) const;
    uint32_t getHighestLevel(void) const;
    void bringFormToFront(uint32_t formIndex);

    // windows size
    std::uint32_t m_windowWidth  = 1920;
    std::uint32_t m_windowHeight = 1080;

    // Dropdown tracking
    int32_t m_expandedDropdownForm    = -1;
    int32_t m_expandedDropdownElement = -1;

    float getSliderKnobX(const sUIElement& slider) const;

    int32_t m_activeElement  = -1;
    int32_t m_activeForm     = -1;
    bool    m_sliderDragging = false;

    // Title bar dragging
    int32_t   m_draggingForm     = -1;
    int32_t   m_draggingElement  = -1;
    bool      m_titleBarDragging = false;
    glm::vec2 m_dragStartMouse   = {};
    glm::vec2 m_dragStartFormPos = {};

    float m_prevMouseX = 0.0f, m_prevMouseY = 0.0f;

    // Hover sound tracking
    int32_t m_lastHoveredForm    = -1;
    int32_t m_lastHoveredElement = -1;

    void updateDropdownBarTexture(sUIForm& form, sUIElement& dropdown);
    void updateButtonTexture(sUIForm& form, sUIElement& button, const std::string& newText);
};

#endif // UI_SYSTEM_HPP
