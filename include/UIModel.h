#pragma once
#include <string>

/** Simple enum to represent autopilot states. */
enum class UIAutoMode {
    STANDBY,
    AUTO
};

/** 
 * Data structure representing all the UI state 
 * the view needs to display.
 */
struct UIState {
    UIAutoMode autoMode;          // e.g. STANDBY or AUTO
    std::string currentSteeringMode;  // e.g. "OFF", "TRACK_HEADING", etc.
    float headingSetpoint;        // numeric setpoint
    float stepSizeSmall;          // small increment
    float stepSizeLarge;          // large increment
};

class UIModel {
public:
    UIModel();
    ~UIModel() = default;

    const UIState& getState() const;

    void setAutoMode(UIAutoMode mode);
    void setSteeringMode(const std::string& modeName);
    void setHeadingSetpoint(float val);

    void incrementSetpointSmall();
    void decrementSetpointSmall();
    void incrementSetpointLarge();
    void decrementSetpointLarge();

private:
    UIState _state;
};
