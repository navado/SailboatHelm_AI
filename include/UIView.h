#pragma once
#include <string>
#include "UIModel.h"

/**
 * A platform-agnostic "View" interface for a 128x256 display. 
 * We assume an underlying driver library (e.g. u8g2),
 * but we do NOT #include Arduino.h or other Arduino specifics here.
 */
class UIView {
public:
    UIView();
    ~UIView();

    // Initialize the display driver
    bool begin();

    // Render the UI state
    void render(const UIModel& model);

private:
    std::string autoModeToStr(UIAutoMode mode) const;

    // Opaque pointer or reference to your chosen display driver
    // We won't reference Arduino calls here. We'll assume the .cpp
    // handles the library. 
    void* _u8g2; // We can cast to the real type in the .cpp
};
