#include "UIView.h"
#include <U8g2lib.h>  // real u8g2 library
#include <U8x8lib.h>  // additional u8x8 library for display constructors
#include <Wire.h>

UIView::UIView()
: _u8g2(nullptr)
{
}

UIView::~UIView() {
    if (_u8g2) {
        delete _u8g2;
        _u8g2 = nullptr;
    }
}

/**
 * For a 128x256 display, pick an appropriate constructor from u8g2lib
 * Example: U8G2_ST75256_JLX256_F_HW_I2C for 128x256
 * (Verify if that constructor is valid for your hardware)
 */
bool UIView::begin() {
    // Create an instance dynamically
    // This is an example constructor; adapt it to your display driver
    _u8g2 = new U8G2_UC1628_256X128_2_4W_HW_SPI(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    // Initialize
    _u8g2->begin();
    // Possibly set contrast or rotation
    _u8g2->setContrast(200);  // example
    return true;
}

void UIView::render(const UIModel& model) {
    if(!_u8g2) return;

    // Start drawing
    _u8g2->clearBuffer();

    // Display auto/standby
    String autoModeStr = autoModeToStr(model.getState().autoMode);
    _u8g2->setFont(u8g2_font_6x10_tr);
    _u8g2->drawStr(0, 10, ("AutoMode: " + autoModeStr).c_str());

    // Display steering mode
    _u8g2->drawStr(0, 25, ("SteerMode: " + model.getState().currentSteeringMode).c_str());

    // Display heading setpoint
    char headingBuf[32];
    snprintf(headingBuf, sizeof(headingBuf), "Setpoint: %.1f deg", (double)model.getState().headingSetpoint);
    _u8g2->drawStr(0, 40, headingBuf);

    // Info line
    _u8g2->drawStr(0, 55, "(Btns: +/- small, << >> large, Mode toggle)");

    // Send to display
    _u8g2->sendBuffer();
}

String UIView::autoModeToStr(UIAutoMode mode) const {
    return (mode == UIAutoMode::AUTO) ? "AUTO" : "STANDBY";
}
