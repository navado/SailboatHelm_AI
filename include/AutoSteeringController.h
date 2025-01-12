#pragma once
#include <string>

/** Simple autopilot modes. */
enum  AutoSteeringMode {
    OFF,
    TRACK_HEADING,
    TRACK_COURSE,
    TRACK_WIND_ANGLE
};

class AutoSteeringController {
public:
    AutoSteeringController();
    ~AutoSteeringController() = default;

    // Set autopilot mode + param (like desired heading)
    void setMode(AutoSteeringMode mode, float param=0.0f);

    // The main update function
    void update(float dt);

    // Return the desired rudder angle
    float getRudderAngle() const;

private:
    void computeSteering(float dt);

    AutoSteeringMode _mode;
    float _desiredHeading;
    float _desiredCourse;
    float _desiredWindAngle;
    float _rudderAngle;

    // A small PID or P-control
    float _kP, _kI, _kD;
    float _integral, _lastError;
};
