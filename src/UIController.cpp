#include "UIController.h"

UIController::UIController(UIModel& model,
                           AutoSteeringController& autoSteer,
                           const IInputDevice& input)
: _model(model)
, _autoSteer(autoSteer)
, _input(input)
, _lastAuto(false)
, _lastMode(false)
{}

void UIController::update() {
    readButtons();
}

void UIController::readButtons() {
    bool autoBtn = _input.isPressed(ButtonId::BTN_AUTO);
    bool modeBtn = _input.isPressed(ButtonId::BTN_MODE);
    bool incS    = _input.isPressed(ButtonId::BTN_INC_SMALL);
    bool decS    = _input.isPressed(ButtonId::BTN_DEC_SMALL);
    bool incL    = _input.isPressed(ButtonId::BTN_INC_LARGE);
    bool decL    = _input.isPressed(ButtonId::BTN_DEC_LARGE);

    // Toggle STANDBY/AUTO on rising edge
    if(autoBtn && !_lastAuto) {
        if(_model.getState().autoMode == UIAutoMode::STANDBY) {
            _model.setAutoMode(UIAutoMode::AUTO);
            _autoSteer.setMode(AutoSteeringMode::TRACK_HEADING,
                               _model.getState().headingSetpoint);
        } else {
            _model.setAutoMode(UIAutoMode::STANDBY);
            _autoSteer.setMode(AutoSteeringMode::OFF);
        }
    }
    _lastAuto = autoBtn;

    // Cycle steering mode on rising edge
    if(modeBtn && !_lastMode) {
        cycleSteeringMode();
    }
    _lastMode = modeBtn;

    if(incS) {
        _model.incrementSetpointSmall();
        updateAutoSteerSetpoint();
    }
    if(decS) {
        _model.decrementSetpointSmall();
        updateAutoSteerSetpoint();
    }
    if(incL) {
        _model.incrementSetpointLarge();
        updateAutoSteerSetpoint();
    }
    if(decL) {
        _model.decrementSetpointLarge();
        updateAutoSteerSetpoint();
    }
}

void UIController::cycleSteeringMode() {
    static std::string modes[] = {
        "OFF",
        "TRACK_HEADING",
        "TRACK_COURSE",
        "TRACK_WIND_ANGLE"
    };
    static int idx = 0;
    idx = (idx + 1) % 4;

    _model.setSteeringMode(modes[idx]);
    if(modes[idx] == "OFF") {
        _autoSteer.setMode(AutoSteeringMode::OFF);
    } else if(modes[idx] == "TRACK_HEADING") {
        _autoSteer.setMode(AutoSteeringMode::TRACK_HEADING,
                           _model.getState().headingSetpoint);
    } else if(modes[idx] == "TRACK_COURSE") {
        _autoSteer.setMode(AutoSteeringMode::TRACK_COURSE, 120.0f);
    } else if(modes[idx] == "TRACK_WIND_ANGLE") {
        _autoSteer.setMode(AutoSteeringMode::TRACK_WIND_ANGLE, 45.0f);
    }
}

void UIController::updateAutoSteerSetpoint() {
    if(_model.getState().autoMode == UIAutoMode::AUTO) {
        float sp = _model.getState().headingSetpoint;
        const std::string& mode = _model.getState().currentSteeringMode;
        if(mode=="OFF") {
            _autoSteer.setMode(AutoSteeringMode::OFF);
        } else if(mode=="TRACK_HEADING") {
            _autoSteer.setMode(AutoSteeringMode::TRACK_HEADING, sp);
        } else if(mode=="TRACK_COURSE") {
            _autoSteer.setMode(AutoSteeringMode::TRACK_COURSE, sp);
        } else if(mode=="TRACK_WIND_ANGLE") {
            _autoSteer.setMode(AutoSteeringMode::TRACK_WIND_ANGLE, sp);
        }
    }
}
