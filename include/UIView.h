#pragma once
#include <Arduino.h>
#include "UIModel.h"

// Forward-declare a pointer or reference to your u8g2 object if you want
// or you can create it in the .cpp
class U8G2;

/**
 * UIView: The "View" in MVC for a 128x256 display using u8g2.
 * Responsible for drawing the UI.
 */
class UIView {
public:
    UIView();
    ~UIView();

    // Initialize the display (u8g2, etc.)
    bool begin();

    // Render the UI from the model
    void render(const UIModel& model);

private:
    // Some helper to convert UIAutoMode to a string
    String autoModeToStr(UIAutoMode mode) const;
    // Our u8g2 display handle
    U8G2* _u8g2;
};
