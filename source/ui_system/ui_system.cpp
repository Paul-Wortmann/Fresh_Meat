// ============================================================================
// ui_system.cpp
// ============================================================================

#include "ui_system.hpp"

#ifndef FILE_PATH_SOUND
    #define FILE_PATH_SOUND   "data/sound/"
#endif

#ifndef FILE_PATH_TEXTURE
    #define FILE_PATH_TEXTURE "data/texture/"
#endif

bool cUISystem::initialize(void)
{
    m_lastHoveredForm    = -1;
    m_lastHoveredElement = -1;
    return true;
}

void cUISystem::terminate(void)
{
    m_forms.clear();
    m_freeList.clear();

    for (sUIEvent* tEvent = getEvent(); tEvent != nullptr; tEvent = getEvent())
        delete tEvent;
}

uint32_t cUISystem::getNewForm(void)
{
    if (!m_freeList.empty())
    {
        uint32_t index = m_freeList.back();
        m_freeList.pop_back();
        m_forms[index] = sUIForm{};
        m_forms[index].enabled = true;
        return index;
    }
    m_forms.emplace_back();
    m_forms.back().enabled = true;
    return static_cast<uint32_t>(m_forms.size() - 1);
}

void cUISystem::destroyForm(const uint32_t& _index)
{
    if (_index >= m_forms.size() || !m_forms[_index].enabled) return;
    if (m_activeForm == static_cast<int32_t>(_index)) { m_activeForm = -1; m_activeElement = -1; }
    if (m_expandedDropdownForm == static_cast<int32_t>(_index))
        m_expandedDropdownForm = m_expandedDropdownElement = -1;
    if (m_draggingForm == static_cast<int32_t>(_index))
        m_titleBarDragging = false, m_draggingForm = m_draggingElement = -1;
    m_forms[_index].elements.clear();
    m_forms[_index].enabled = false;
    m_freeList.push_back(_index);
}

void cUISystem::setFormEnabled(const int32_t& _formID, const bool& _state)
{
    if (_formID < 0 || static_cast<size_t>(_formID) >= m_forms.size()) return;
    if (m_forms[_formID].enabled == _state) return;

    if (!_state && m_expandedDropdownForm == _formID)
    {
        auto& dropdown = m_forms[_formID].elements[m_expandedDropdownElement];
        dropdown.expanded = false;
        dropdown.hoveredOptionIndex = -1;
        m_expandedDropdownForm = m_expandedDropdownElement = -1;
    }
    if (!_state && m_draggingForm == _formID)
        m_titleBarDragging = false, m_draggingForm = m_draggingElement = -1;

    m_forms[_formID].enabled = _state;
}

bool cUISystem::getFormEnabled(const int32_t& _formID)
{
    if (_formID < 0 || static_cast<size_t>(_formID) >= m_forms.size())
        return false;
    return m_forms[_formID].enabled;
}

bool cUISystem::isMouseInside(const glm::vec2& _pos, const glm::vec2& _dimensions, const glm::vec2& _mousePos) const
{
    return (_mousePos.x >= _pos.x && _mousePos.x <= _pos.x + _dimensions.x &&
            _mousePos.y >= _pos.y && _mousePos.y <= _pos.y + _dimensions.y);
}

uint32_t cUISystem::getHighestLevel(void) const
{
    uint32_t maxLevel = 0;
    for (const auto& form : m_forms)
        if (form.enabled) maxLevel = std::max(maxLevel, form.level);
    return maxLevel;
}

void cUISystem::bringFormToFront(uint32_t formIndex)
{
    if (formIndex >= m_forms.size() || !m_forms[formIndex].enabled) return;
    uint32_t newLevel = getHighestLevel() + 1;
    m_forms[formIndex].level = newLevel;
}

float cUISystem::getSliderKnobX(const sUIElement& slider) const
{
    float t = slider.elementValue / 100.0f;
    return slider.position.x + t * slider.dimensions.x - slider.dimensionsExt.x * 0.5f;
}

void cUISystem::updateDropdownBarTexture(sUIForm& form, sUIElement& dropdown)
{
    if (dropdown.options.empty() || dropdown.selectedIndex < 0 ||
        static_cast<size_t>(dropdown.selectedIndex) >= dropdown.options.size())
        return;
    std::string selectedText = dropdown.options[dropdown.selectedIndex];
    int32_t newTex = m_graphicsSystem->generateTexture(dropdown.dimensions.x, selectedText);
    if (newTex != -1) dropdown.textTextureID = newTex;
}

void cUISystem::updateButtonTexture(sUIForm& form, sUIElement& button, const std::string& newText)
{
    if (button.type != eUIElementType::button) return;
    int32_t newTex = m_graphicsSystem->generateTexture(button.dimensions.x, newText);
    if (newTex != -1) button.textTextureID = newTex;
}

void cUISystem::setProgressBar(const int32_t& _formID, const int32_t& _elementID, const float& _value)
{
    if (_formID < 0 || static_cast<size_t>(_formID) >= m_forms.size()) return;
    auto& form = m_forms[_formID];
    if (_elementID < 0 || static_cast<size_t>(_elementID) >= form.elements.size()) return;
    form.elements[_elementID].elementValue = std::clamp(_value, 0.0f, 100.0f);
}

void cUISystem::setSliderValue(const int32_t& _formID, const int32_t& _elementID, const float& _value)
{
    if (_formID < 0 || static_cast<size_t>(_formID) >= m_forms.size()) return;
    auto& form = m_forms[_formID];
    if (_elementID < 0 || static_cast<size_t>(_elementID) >= form.elements.size()) return;
    auto& el = form.elements[_elementID];
    if (el.type != eUIElementType::slider) return;
    el.elementValue = std::clamp(_value, 0.0f, 100.0f);
}

void cUISystem::setDropdownSelection(const int32_t& _formID, const int32_t& _elementID, const int32_t& _index)
{
    if (_formID < 0 || static_cast<size_t>(_formID) >= m_forms.size()) return;
    auto& form = m_forms[_formID];
    if (_elementID < 0 || static_cast<size_t>(_elementID) >= form.elements.size()) return;
    auto& el = form.elements[_elementID];
    if (el.type != eUIElementType::dropdown) return;
    if (_index >= 0 && static_cast<size_t>(_index) < el.options.size())
    {
        el.selectedIndex = _index;
        updateDropdownBarTexture(form, el);
    }
}

void cUISystem::setButtonText(const int32_t& _formID, const int32_t& _elementID, const std::string& _text)
{
    if (_formID < 0 || static_cast<size_t>(_formID) >= m_forms.size()) return;
    auto& form = m_forms[_formID];
    if (_elementID < 0 || static_cast<size_t>(_elementID) >= form.elements.size()) return;
    auto& el = form.elements[_elementID];
    if (el.type != eUIElementType::button) return;
    updateButtonTexture(form, el, _text);
}

void cUISystem::setTitleBarText(const int32_t& _formID, const int32_t& _elementID, const std::string& _text)
{
    if (_formID < 0 || static_cast<size_t>(_formID) >= m_forms.size()) return;
    auto& form = m_forms[_formID];
    if (_elementID < 0 || static_cast<size_t>(_elementID) >= form.elements.size()) return;
    auto& el = form.elements[_elementID];
    if (el.type != eUIElementType::titleBar) return;
    int32_t newTex = m_graphicsSystem->generateTexture(el.dimensions.x, _text);
    if (newTex != -1) el.textTextureID = newTex;
}

int32_t cUISystem::getFormIndexByName(const std::string& _name) const
{
    for (size_t i = 0; i < m_forms.size(); ++i)
        if (m_forms[i].enabled && m_forms[i].name == _name)
            return static_cast<int32_t>(i);
    return -1;
}

bool cUISystem::loadUI(const std::string& _fileName)
{
    auto extractString = [](const std::string& line) -> std::string {
        size_t start = line.find('>'), end = line.rfind('<');
        if (start == std::string::npos || end == std::string::npos || start + 1 >= end) return {};
        return line.substr(start + 1, end - start - 1);
    };
    auto extractVec2 = [&](const std::string& line) -> glm::vec2 {
        std::stringstream ss(extractString(line)); float x, y; char comma;
        if (!(ss >> x >> comma >> y)) return {};
        return {x, y};
    };
    auto extractVec3 = [&](const std::string& line) -> glm::vec3 {
        std::stringstream ss(extractString(line)); float x, y, z; char comma;
        if (!(ss >> x >> comma >> y >> comma >> z)) return {};
        return {x, y, z};
    };
    auto extractVec4 = [&](const std::string& line) -> glm::vec4 {
        std::stringstream ss(extractString(line)); float r, g, b, a; char comma;
        if (!(ss >> r >> comma >> g >> comma >> b >> comma >> a)) return {1,1,1,1};
        return {r, g, b, a};
    };
    auto extractFloat = [&](const std::string& line) -> float { return std::stof(extractString(line)); };
    auto extractInt = [&](const std::string& line) -> int32_t { return std::stoi(extractString(line)); };

    std::ifstream file(_fileName);
    if (!file.is_open()) { std::cout << "Failed to load file: " << _fileName << std::endl; return false; }

    int32_t formIndex = -1, elementIndex = -1;
    std::string line;
    bool formCentered = false;
    while (std::getline(file, line))
    {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line == "<form>")
        {
            formIndex = getNewForm();
            m_forms[formIndex].id = formIndex;
            elementIndex = -1;
            formCentered = false;
            continue;
        }
        else if (line == "</form>")
        {
            if (formCentered)
            {
                std::uint32_t centerX = (m_windowWidth - m_forms[formIndex].dimensions.x) * 0.5f;
                std::uint32_t centerY = (m_windowHeight - m_forms[formIndex].dimensions.y) * 0.5f;
                m_forms[formIndex].position = glm::vec3(centerX, centerY, 0.0f);
            }
            formIndex = -1;
            continue;
        }
        else if (line == "<element>")
        {
            m_forms[formIndex].elements.emplace_back();
            elementIndex = (int32_t)m_forms[formIndex].elements.size() - 1;
            auto& el = m_forms[formIndex].elements[elementIndex];
            el.enabled = true;
            el.id = elementIndex;
            continue;
        }

        size_t tag_start = line.find('<'), tag_end = line.find('>');
        if (tag_start == std::string::npos || tag_end == std::string::npos) continue;
        std::string tag = line.substr(tag_start + 1, tag_end - tag_start - 1);

        // global colors
        if (tag == "color_text") m_uiColor.colorText = extractVec4(line);
        else if (tag == "color_background") m_uiColor.colorBackground = extractVec4(line);
        else if (tag == "color_illuminated") m_uiColor.colorIlluminated = extractVec4(line);
        else if (tag == "color_hovered") m_uiColor.colorHovered = extractVec4(line);
        else if (tag == "color_pressed") m_uiColor.colorPressed = extractVec4(line);
        else if (tag == "color_clicked") m_uiColor.colorClicked = extractVec4(line);
        else if (tag == "color_default") m_uiColor.colorDefault = extractVec4(line);

        // global audio
        else if (tag == "audio_hovered")
        {
            std::string fileName = extractString(line);
            m_uiAudio.hoverID = m_audioSystem->loadSound(FILE_PATH_SOUND + fileName);
        }
        else if (tag == "audio_clicked")
        {
            std::string fileName = extractString(line);
            m_uiAudio.clickID = m_audioSystem->loadSound(FILE_PATH_SOUND + fileName);
        }

        if (formIndex != -1 && elementIndex == -1)
        {
            // form properties
            if (tag == "form_name") m_forms[formIndex].name = extractString(line);
            else if (tag == "form_enabled") m_forms[formIndex].enabled = (extractInt(line) != 0);
            else if (tag == "form_dimensions") m_forms[formIndex].dimensions = extractVec2(line);
            else if (tag == "form_position") m_forms[formIndex].position = extractVec3(line);
            else if (tag == "form_centered") formCentered = (extractInt(line) != 0);
            else if (tag == "form_texture_base")
                m_forms[formIndex].textureID = m_graphicsSystem->loadTexture(std::string(FILE_PATH_TEXTURE) + extractString(line));
        }
        else if (formIndex != -1 && elementIndex != -1)
        {
            auto& el = m_forms[formIndex].elements[elementIndex];
            if (tag == "element_type") {
                el.type = static_cast<eUIElementType>(extractInt(line));
                if (el.type == eUIElementType::text || el.type == eUIElementType::progressBar)
                    el.interactive = false;
                if (el.type == eUIElementType::titleBar)
                    el.interactive = true; // for dragging only
            }
            else if (tag == "element_enabled") el.enabled = (extractInt(line) != 0);
            else if (tag == "element_dimensions") el.dimensions = extractVec2(line);
            else if (tag == "element_dimensions_ext") el.dimensionsExt = extractVec2(line);
            else if (tag == "element_position") el.position = extractVec3(line);
            else if (tag == "element_texture_base")
                el.textureBaseID = m_graphicsSystem->loadTexture(std::string(FILE_PATH_TEXTURE) + extractString(line));
            else if (tag == "element_texture_ext")
                el.textureExtID = m_graphicsSystem->loadTexture(std::string(FILE_PATH_TEXTURE) + extractString(line));
            else if (tag == "element_text") {
                std::string text = extractString(line);
                el.textTextureID = m_graphicsSystem->generateTexture(el.dimensions.x, text);
            }
            else if (tag == "dropdown_option") {
                std::string opt = extractString(line);
                el.options.push_back(opt);
                int32_t texID = m_graphicsSystem->generateTexture(el.dimensions.x, opt);
                el.optionTextures.push_back(texID);
            }
            else if (tag == "element_value") {
                if (el.type == eUIElementType::dropdown) {
                    int32_t idx = extractInt(line);
                    if (idx >= 0 && idx < (int32_t)el.options.size()) {
                        el.selectedIndex = idx;
                        updateDropdownBarTexture(m_forms[formIndex], el);
                    }
                } else {
                    el.elementValue = std::clamp(extractFloat(line), 0.0f, 100.0f);
                }
            }
        }
    }
    return true;
}

static bool isPointInsideDropdownExpanded(const sUIForm& form, const sUIElement& dropdown, const glm::vec2& point)
{
    glm::vec2 barPos = glm::vec2(form.position) + glm::vec2(dropdown.position);
    float totalHeight = dropdown.dimensions.y * (1 + dropdown.options.size());
    return (point.x >= barPos.x && point.x <= barPos.x + dropdown.dimensions.x &&
            point.y >= barPos.y && point.y <= barPos.y + totalHeight);
}

static int getOptionIndexAtPoint(const sUIForm& form, const sUIElement& dropdown, const glm::vec2& point)
{
    if (!dropdown.expanded) return -1;
    glm::vec2 barPos = glm::vec2(form.position) + glm::vec2(dropdown.position);
    float optionHeight = dropdown.dimensions.y;
    for (size_t i = 0; i < dropdown.options.size(); ++i)
    {
        glm::vec2 optPos = barPos + glm::vec2(0.0f, optionHeight * (i + 1));
        if (point.x >= optPos.x && point.x <= optPos.x + dropdown.dimensions.x &&
            point.y >= optPos.y && point.y <= optPos.y + optionHeight)
            return static_cast<int>(i);
    }
    return -1;
}

bool cUISystem::process(float _delta)
{
    glm::vec2 mouse = { m_io->mouseX, m_io->mouseY };
    bool leftPressed   = m_io->mouseLeftPressed;
    bool leftReleased  = m_io->mouseLeftReleased;

    // 1. Find topmost form
    int32_t topFormIndex = -1;
    uint32_t highestLevel = 0;
    for (uint32_t i = 0; i < m_forms.size(); ++i)
    {
        auto& form = m_forms[i];
        if (!form.enabled) continue;
        if (!isMouseInside(glm::vec2(form.position), form.dimensions, mouse)) continue;
        if (topFormIndex == -1 || form.level > highestLevel)
        {
            topFormIndex = i;
            highestLevel = form.level;
        }
        else if (form.level == highestLevel && i > static_cast<uint32_t>(topFormIndex))
        {
            topFormIndex = i;
        }
    }

    bool mouseOverUI = (topFormIndex != -1);

    // Reset element states
    for (auto& form : m_forms)
    {
        if (!form.enabled) continue;
        for (auto& el : form.elements)
        {
            el.state = el.enabled ? eUIElementState::none : eUIElementState::disabled;
            if (el.type == eUIElementType::dropdown && !el.expanded)
                el.hoveredOptionIndex = -1;
        }
    }

    // 2. Collapse expanded dropdown if click outside
    bool dropdownCollapsed = false;
    if (leftPressed && m_expandedDropdownForm != -1 && m_expandedDropdownElement != -1)
    {
        auto& form = m_forms[m_expandedDropdownForm];
        auto& dropdown = form.elements[m_expandedDropdownElement];
        if (!isPointInsideDropdownExpanded(form, dropdown, mouse))
        {
            dropdown.expanded = false;
            dropdown.hoveredOptionIndex = -1;
            m_expandedDropdownForm = -1;
            m_expandedDropdownElement = -1;
            dropdownCollapsed = true;
        }
    }

    // 3. Bring clicked form to front (if not collapsing a dropdown)
    if (leftPressed && mouseOverUI && !dropdownCollapsed)
        bringFormToFront(static_cast<uint32_t>(topFormIndex));

    // 4. Find topmost interactive element on the top form
    int32_t topElementIndex = -1;
    if (mouseOverUI)
    {
        auto& form = m_forms[topFormIndex];

        if (m_expandedDropdownForm != -1 && m_expandedDropdownForm == topFormIndex)
        {
            auto& dropdown = m_forms[m_expandedDropdownForm].elements[m_expandedDropdownElement];
            if (dropdown.enabled && dropdown.interactive &&
                isPointInsideDropdownExpanded(m_forms[m_expandedDropdownForm], dropdown, mouse))
                topElementIndex = m_expandedDropdownElement;
        }

        if (topElementIndex == -1)
        {
            for (int32_t i = (int32_t)form.elements.size() - 1; i >= 0; --i)
            {
                auto& el = form.elements[i];
                if (!el.enabled || !el.interactive) continue;
                bool hit = false;
                if (el.type == eUIElementType::dropdown && el.expanded)
                    hit = isPointInsideDropdownExpanded(form, el, mouse);
                else
                {
                    glm::vec2 worldPos = glm::vec2(form.position) + glm::vec2(el.position);
                    hit = isMouseInside(worldPos, el.dimensions, mouse);
                }
                if (hit) { topElementIndex = i; break; }
            }
        }

        // --- TITLE BAR DRAGGING (overrides normal interaction) ---
        bool isTitleBar = (topElementIndex != -1 && form.elements[topElementIndex].type == eUIElementType::titleBar);
        if (isTitleBar)
        {
            auto& titleBar = form.elements[topElementIndex];
            if (leftPressed && !m_titleBarDragging)
            {
                // Start drag
                m_titleBarDragging = true;
                m_draggingForm = topFormIndex;
                m_draggingElement = topElementIndex;
                m_dragStartMouse = mouse;
                m_dragStartFormPos = glm::vec2(form.position);
                titleBar.state = eUIElementState::pressed;
            }

            if (m_titleBarDragging && m_draggingForm == topFormIndex)
            {
                if (leftReleased)
                {
                    // Stop drag
                    m_titleBarDragging = false;
                    m_draggingForm = -1;
                    m_draggingElement = -1;
                    titleBar.state = eUIElementState::none;
                }
                else
                {
                    // Update form position every frame while dragging
                    glm::vec2 delta = mouse - m_dragStartMouse;
                    glm::vec3 newPos = glm::vec3(m_dragStartFormPos + delta, form.position.z);
                    form.position = newPos;
                    titleBar.state = eUIElementState::pressed;
                }
            }
            // Skip normal element processing for title bar
            topElementIndex = -1;
        }

        // Hovered option index for expanded dropdown
        if (topElementIndex != -1 && form.elements[topElementIndex].type == eUIElementType::dropdown && form.elements[topElementIndex].expanded)
            form.elements[topElementIndex].hoveredOptionIndex = getOptionIndexAtPoint(form, form.elements[topElementIndex], mouse);

        // Normal element interaction (buttons, sliders, dropdowns)
        if (leftPressed && topElementIndex != -1 && !dropdownCollapsed)
        {
            auto& el = form.elements[topElementIndex];
            if (el.type == eUIElementType::dropdown)
            {
                int optionIdx = getOptionIndexAtPoint(form, el, mouse);
                if (optionIdx != -1)
                {
                    el.selectedIndex = optionIdx;
                    updateDropdownBarTexture(form, el);
                    el.expanded = false;
                    el.hoveredOptionIndex = -1;
                    m_expandedDropdownForm = -1;
                    m_expandedDropdownElement = -1;
                    sUIEvent* ev = new sUIEvent;
                    ev->type = eUIEventType::dropdownSelectionChanged;
                    ev->form = topFormIndex;
                    ev->element = topElementIndex;
                    ev->selection = optionIdx;
                    m_event.push(ev);
                }
                else
                {
                    if (m_expandedDropdownForm != -1 && m_expandedDropdownElement != -1)
                    {
                        auto& otherForm = m_forms[m_expandedDropdownForm];
                        auto& otherDropdown = otherForm.elements[m_expandedDropdownElement];
                        otherDropdown.expanded = false;
                        otherDropdown.hoveredOptionIndex = -1;
                    }
                    el.expanded = !el.expanded;
                    if (el.expanded)
                    {
                        m_expandedDropdownForm = topFormIndex;
                        m_expandedDropdownElement = topElementIndex;
                    }
                    else
                    {
                        el.hoveredOptionIndex = -1;
                        m_expandedDropdownForm = -1;
                        m_expandedDropdownElement = -1;
                    }
                }
                m_activeElement = -1;
                m_activeForm = -1;
            }
            else if (el.type == eUIElementType::slider)
            {
                m_activeElement = topElementIndex;
                m_activeForm = topFormIndex;
                m_sliderDragging = true;
                el.state = eUIElementState::pressed;
            }
            else if (el.type == eUIElementType::button)
            {
                m_activeElement = topElementIndex;
                m_activeForm = topFormIndex;
                el.state = eUIElementState::pressed;
            }
        }

        // Slider dragging
        if (m_sliderDragging && m_activeForm != -1)
        {
            auto& dragForm = m_forms[m_activeForm];
            auto& dragEl = dragForm.elements[m_activeElement];
            if (dragEl.type == eUIElementType::slider)
            {
                glm::vec2 worldPos = glm::vec2(dragForm.position) + glm::vec2(dragEl.position);
                float rel = (mouse.x - worldPos.x) / dragEl.dimensions.x;
                float newValue = std::clamp(rel * 100.0f, 0.0f, 100.0f);
                if (dragEl.elementValue != newValue)
                {
                    dragEl.elementValue = newValue;
                    sUIEvent* ev = new sUIEvent;
                    ev->type = eUIEventType::sliderValueChanged;
                    ev->form = m_activeForm;
                    ev->element = m_activeElement;
                    ev->value = newValue;
                    m_event.push(ev);
                }
                dragEl.state = eUIElementState::pressed;
            }
        }

        // Button click release + click sound
        if (leftReleased)
        {
            if (m_activeForm != -1 && m_activeElement != -1 &&
                m_activeForm == topFormIndex && m_activeElement == topElementIndex)
            {
                auto& relForm = m_forms[topFormIndex];
                auto& relEl = relForm.elements[topElementIndex];
                if (relEl.type == eUIElementType::button)
                {
                    // Play click sound
                    if (m_uiAudio.clickID != -1)
                        m_audioSystem->playSound(m_uiAudio.clickID);

                    sUIEvent* ev = new sUIEvent;
                    ev->type = eUIEventType::buttonClicked;
                    ev->form = m_activeForm;
                    ev->element = m_activeElement;
                    m_event.push(ev);
                }
            }
            m_sliderDragging = false;
            m_activeElement = -1;
            m_activeForm = -1;
        }

        // Hover state for non-active, non-title-bar elements – play sound only on transition
        if (topElementIndex != -1 && m_activeElement == -1 && !isTitleBar)
        {
            auto& el = form.elements[topElementIndex];
            if (el.type != eUIElementType::progressBar)
            {
                // Check if this is a new hover (different form or element)
                bool hoverChanged = (topFormIndex != m_lastHoveredForm ||
                                     topElementIndex != m_lastHoveredElement);
                if (hoverChanged && el.state != eUIElementState::hovered)
                {
                    if (m_uiAudio.hoverID != -1)
                        m_audioSystem->playSound(m_uiAudio.hoverID);
                }
                el.state = eUIElementState::hovered;
                m_lastHoveredForm = topFormIndex;
                m_lastHoveredElement = topElementIndex;
            }
        }
        else
        {
            // No hover – reset tracking
            if (m_lastHoveredForm != -1 || m_lastHoveredElement != -1)
            {
                m_lastHoveredForm = -1;
                m_lastHoveredElement = -1;
            }
        }
    }

    // Clear hovered option if mouse leaves expanded dropdown
    if (m_expandedDropdownForm != -1 && m_expandedDropdownElement != -1)
    {
        auto& form = m_forms[m_expandedDropdownForm];
        auto& dropdown = form.elements[m_expandedDropdownElement];
        if (!isPointInsideDropdownExpanded(form, dropdown, mouse))
            dropdown.hoveredOptionIndex = -1;
    }

    m_prevMouseX = m_io->mouseX;
    m_prevMouseY = m_io->mouseY;
    return mouseOverUI;
}
