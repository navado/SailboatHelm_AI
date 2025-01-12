#include "UIView.h"
#include <u8g2lib.h>   // but we do not use Arduino.h
#include <cstdio>      // for snprintf

UIView::UIView()
: _u8g2(nullptr)
{
}

UIView::~UIView() {
    if(_u8g2) {
        // cast back to U8G2 pointer
        U8G2* ptr = static_cast<U8G2*>(_u8g2);
        delete ptr;
        _u8g2 = nullptr;
    }
}

bool UIView::begin() {
    // Create an instance dynamically
    // Use your actual 128x256 driver. Example:
    U8G2* u8 = new U8G2_ST75256_JLX19296_2_3W_SW_SPI(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    if(!u8) return false;
    u8->begin();
    _u8g2 = static_cast<void*>(u8);
    return true;
}

void UIView::render(const UIModel& model) {
    if(!_u8g2) return;
    U8G2* u8 = static_cast<U8G2*>(_u8g2);

    u8->clearBuffer();
    u8->setFont(u8g2_font_6x10_tr);

    // 1) Display auto mode
    std::string strAuto = "AutoMode: " + autoModeToStr(model.getState().autoMode);
    u8->drawStr(0, 10, strAuto.c_str());

    // 2) Steering mode
    std::string strMode = "SteerMode: " + model.getState().currentSteeringMode;
    u8->drawStr(0, 25, strMode.c_str());

    // 3) Heading setpoint
    char buf[32];
    std::snprintf(buf, sizeof(buf), "Setpoint: %.1f deg", model.getState().headingSetpoint);
    u8->drawStr(0, 40, buf);

    // 4) final 
    u8->sendBuffer();
}

std::string UIView::autoModeToStr(UIAutoMode mode) const {
    return (mode == UIAutoMode::AUTO) ? "AUTO" : "STANDBY";
}
