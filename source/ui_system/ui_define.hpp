
#ifndef UI_DEFINE_HPP
#define UI_DEFINE_HPP

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

enum class eUIElementType : std::uint32_t
{
    none        = 0,
    text        = 1,
    button      = 2,
    slider      = 3, // horizontal slider
    progressBar = 4, // visual progress bar
    dropdown    = 5, // drop-down selection box
    titleBar    = 6
};

enum class eUIElementState : std::uint32_t
{
    // these states are also used when renderering by a separate system
    none     = 0,
    disabled = 1,
    hovered  = 2,
    pressed  = 3,
    clicked  = 4
};

struct sUIElement
{
    // component
    bool                     enabled             = false;

    // data
    eUIElementType           type                = eUIElementType::none;
    eUIElementState          state               = eUIElementState::none;
    std::uint32_t            id                  = 0;
    bool                     interactive         = true;  // can interact with mouse / keyboard
    glm::vec3                position            = {};    // relative to the form
    glm::vec2                dimensions          = {};    // pixels
    glm::vec2                dimensionsExt       = {};    // pixels, extended uses, slider knob, etc...
    std::int32_t             textureBaseID       = -1;    // main base texture
    std::int32_t             textureExtID        = -1;    // additional texture used for slider knobs or other extended uses
    std::int32_t             textTextureID       = -1;    // generated text texture for buttons/text elements
    float                    elementValue        = 0.0f;  // range 0.0 - 100.0 // slider, progress bar, etc...
    std::vector<std::string> options             = {};    // option names
    std::vector<int32_t>     optionTextures      = {};    // texture IDs for each option
    int32_t                  selectedIndex       = 0;
    bool                     expanded            = false; // drop-down box
    int32_t                  hoveredOptionIndex  = -1;    // index of option being hovered, -1 if none
};

struct sUIForm
{
    // component
    bool              enabled = false;

    // data
    std::string   name             = {};
    std::uint32_t id               = 0;
    std::uint32_t level            = 0;  // higher = closer
    glm::vec3     position         = {}; // pixels
    glm::vec2     dimensions       = {}; // pixels
    std::int32_t  textureID        = -1;

    // elements
    std::vector<sUIElement> elements = {};
};

struct sUIColor
{
    // color
    glm::vec4 colorText        = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 colorBackground  = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 colorIlluminated = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 colorHovered     = {0.8f, 0.8f, 0.2f, 1.0f};
    glm::vec4 colorPressed     = {0.9f, 0.4f, 0.2f, 1.0f};
    glm::vec4 colorClicked     = {0.2f, 1.0f, 0.2f, 1.0f};
    glm::vec4 colorDefault     = {0.6f, 0.6f, 0.6f, 1.0f};
};

struct sUIAudio
{
    // audio
    std::int32_t hoverID       = -1;
    std::int32_t clickID       = -1;
};

#endif // UI_DEFINE_HPP
