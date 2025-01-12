#pragma once
#include <Arduino.h>
#include "UIModel.h"

// Suppose we have an AutoSteeringController class in your code:
#include "AutoSteeringController.h"

/**
 * The "Controller" in MVC: handles user input (buttons),
 * modifies the UIModel, and updates the autopilot as needed.
 */
class UIController {
public:
    UIController(UIModel& model, AutoSteeringController& autoSteer);

    // Initialize pins if needed
    void begin();

    // Called periodically to check buttons and update model
    void update();

private:
    void readButtons();
    void cycleSteeringMode();
    void updateAutoSteerSetpoint();

    UIModel& _model;
    AutoSteeringController& _autoSteer;

    // Example pins
    static const int PIN_BTN_AUTO  = 2; 
    static const int PIN_BTN_MODE  = 3; 
    static const int PIN_BTN_INC_S = 4; // small increment
    static const int PIN_BTN_DEC_S = 5; // small decrement
    static const int PIN_BTN_INC_L = 6; // large increment
    static const int PIN_BTN_DEC_L = 7; // large decrement

    // We'll track button states to detect edges
    bool _lastAuto;
    bool _lastMode;
};
