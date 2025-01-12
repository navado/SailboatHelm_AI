#pragma once
#include <Arduino.h>
#include <string>

// For demonstration, we define two states for the overall autopilot "mode"
enum class UIAutoMode {
    STANDBY,
    AUTO
};

struct UIState {
    UIAutoMode autoMode;         // Is autopilot in STANDBY or AUTO
    String currentSteeringMode;  // e.g. "OFF", "TRACK_HEADING", etc.
    float headingSetpoint;       // The numeric setpoint we display
    float stepSizeSmall;         // Fine step
    float stepSizeLarge;         // Coarse step
};

/**
 * UIModel: The "Model" in MVC that holds the UI state:
 * - auto/standby
 * - steering mode name
 * - heading setpoint
 * - step sizes for increments
 */
class UIModel {
public:
    UIModel();
    ~UIModel() = default;

    const UIState& getState() const;

    void setAutoMode(UIAutoMode mode);
    void setSteeringMode(const String& modeName);
    void setHeadingSetpoint(float val);

    void incrementSetpointSmall();
    void decrementSetpointSmall();
    void incrementSetpointLarge();
    void decrementSetpointLarge();

private:
    UIState _state;
};
