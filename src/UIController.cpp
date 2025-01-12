#include "UIController.h"

UIController::UIController(UIModel& model, AutoSteeringController& autoSteer)
: _model(model)
, _autoSteer(autoSteer)
, _lastAuto(false)
, _lastMode(false)
{
}

void UIController::begin() {
    pinMode(PIN_BTN_AUTO, INPUT_PULLUP);
    pinMode(PIN_BTN_MODE, INPUT_PULLUP);
    pinMode(PIN_BTN_INC_S, INPUT_PULLUP);
    pinMode(PIN_BTN_DEC_S, INPUT_PULLUP);
    pinMode(PIN_BTN_INC_L, INPUT_PULLUP);
    pinMode(PIN_BTN_DEC_L, INPUT_PULLUP);
}

void UIController::update() {
    readButtons();
}

void UIController::readButtons() {
    // read each button
    bool autoBtn = (digitalRead(PIN_BTN_AUTO) == LOW);
    bool modeBtn = (digitalRead(PIN_BTN_MODE) == LOW);
    bool incS    = (digitalRead(PIN_BTN_INC_S) == LOW);
    bool decS    = (digitalRead(PIN_BTN_DEC_S) == LOW);
    bool incL    = (digitalRead(PIN_BTN_INC_L) == LOW);
    bool decL    = (digitalRead(PIN_BTN_DEC_L) == LOW);

    // Toggle STANDBY/AUTO on rising edge
    if(autoBtn && !_lastAuto) {
        if(_model.getState().autoMode == UIAutoMode::STANDBY) {
            _model.setAutoMode(UIAutoMode::AUTO);
            // Also set autopilot to a default mode
            _autoSteer.setMode(AutoSteeringMode::TRACK_HEADING, _model.getState().headingSetpoint);
        } else {
            _model.setAutoMode(UIAutoMode::STANDBY);
            _autoSteer.setMode(AutoSteeringMode::OFF);
        }
    }
    _lastAuto = autoBtn;

    // Cycle steering modes on rising edge of modeBtn
    if(modeBtn && !_lastMode) {
        cycleSteeringMode();
    }
    _lastMode = modeBtn;

    // Adjust setpoints
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
    // Example: cycle among OFF, TRACK_HEADING, TRACK_WIND_ANGLE, etc.
    static String modes[] = {"OFF", "TRACK_HEADING", "TRACK_COURSE", "TRACK_WIND_ANGLE"};
    static int idx=0;
    idx = (idx+1)%4;

    _model.setSteeringMode(modes[idx]);

    // Also set the autopilot
    if(modes[idx]=="OFF") {
        _autoSteer.setMode(AutoSteeringMode::OFF);
    } else if(modes[idx]=="TRACK_HEADING") {
        _autoSteer.setMode(AutoSteeringMode::TRACK_HEADING, _model.getState().headingSetpoint);
    } else if(modes[idx]=="TRACK_COURSE") {
        _autoSteer.setMode(AutoSteeringMode::TRACK_COURSE, 120.0f); // example
    } else if(modes[idx]=="TRACK_WIND_ANGLE") {
        _autoSteer.setMode(AutoSteeringMode::TRACK_WIND_ANGLE, 45.0f);
    }
}

void UIController::updateAutoSteerSetpoint() {
    // If in AUTO, and currentSteeringMode is e.g. "TRACK_HEADING", etc.
    if(_model.getState().autoMode == UIAutoMode::AUTO) {
        String mode = _model.getState().currentSteeringMode;
        float sp    = _model.getState().headingSetpoint;

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
