#pragma once
#include <string>
#include "UIModel.h"
#include "IInputDevice.h"

// We assume we have an autopilot "AutoSteeringController" that is also
// mostly platform-agnostic
#include "AutoSteeringController.h"

/**
 * The UIController uses the IInputDevice to check button states,
 * then modifies the UIModel and calls the autopilot logic.
 */
class UIController {
public:
    UIController(UIModel& model,
                 AutoSteeringController& autoSteer,
                 const IInputDevice& input);
    ~UIController() = default;

    // Called periodically
    void update();
    void begin();

private:
    void readButtons();
    void cycleSteeringMode();
    void updateAutoSteerSetpoint();

    UIModel& _model;
    AutoSteeringController& _autoSteer;
    const IInputDevice& _input;

    bool _lastAuto;
    bool _lastMode;
};
