
#ifndef UI_EVENT_DEFINE_HPP
#define UI_EVENT_DEFINE_HPP

#include <cstdint>

// there is a separate system which processes and cleans up events.

// Event type enum
enum class eUIEventType : std::uint32_t
{
    none                     = 0,    // null event
    buttonClicked            = 1,    // button clicked
    sliderValueChanged       = 2,    // slider dragged
    dropdownSelectionChanged = 3
};

// Event struct
struct sUIEvent
{
    sUIEvent*     next      = nullptr;
    eUIEventType  type      = eUIEventType::none;
    std::uint32_t form      = 0;
    std::uint32_t element   = 0;
    float         value     = 0.0f; // for element Value Changed
    std::uint32_t selection = 0;    // for selection Changed
};

#endif // UI_EVENT_DEFINE_HPP

