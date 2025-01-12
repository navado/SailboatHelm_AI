#include "UIModel.h"

UIModel::UIModel() {
    // Initialize default state
    _state.autoMode          = UIAutoMode::STANDBY;
    _state.currentSteeringMode = "OFF";
    _state.headingSetpoint   = 90.0f;  // e.g. default to heading=90
    _state.stepSizeSmall     = 1.0f;
    _state.stepSizeLarge     = 10.0f;
}

const UIState& UIModel::getState() const {
    return _state;
}

void UIModel::setAutoMode(UIAutoMode mode) {
    _state.autoMode = mode;
}

void UIModel::setSteeringMode(const String& modeName) {
    _state.currentSteeringMode = modeName;
}

void UIModel::setHeadingSetpoint(float val) {
    _state.headingSetpoint = val;
}

void UIModel::incrementSetpointSmall() {
    _state.headingSetpoint += _state.stepSizeSmall;
}

void UIModel::decrementSetpointSmall() {
    _state.headingSetpoint -= _state.stepSizeSmall;
}

void UIModel::incrementSetpointLarge() {
    _state.headingSetpoint += _state.stepSizeLarge;
}

void UIModel::decrementSetpointLarge() {
    _state.headingSetpoint -= _state.stepSizeLarge;
}
