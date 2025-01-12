#pragma once

#include <Arduino.h>

enum class AutoSteeringMode {
    OFF,
    TRACK_HEADING,
    TRACK_COURSE,
    TRACK_WIND_ANGLE,
    CALIBRATION_PID,
    CALIBRATION_THRESHOLD,
    CALIBRATION_IMU,
    RUDDER_LOCK  // New mode
};

struct EnvironmentData {
    float heading;       // from IMU
    float course;        // from GPS
    float windDirection; // from wind sensor
};

/**
 * Main autopilot logic that produces a *desired rudder angle*.
 */
class AutoSteeringController {
public:
    AutoSteeringController();

    // setMode(RUDDER_LOCK, someAngle) => lock the rudder
    void setMode(AutoSteeringMode mode, float param=0.0f);

    void updateEnvironmentData(const EnvironmentData& env);
    void update();  // do the control logic

    float getDesiredRudderAngle() const;

private:
    void computeSteering();

private:
    AutoSteeringMode _mode;
    EnvironmentData  _env;

    float _desiredHeading;
    float _desiredCourse;
    float _desiredWindAngle;

    // The lock angle if RUDDER_LOCK
    float _lockAngle;

    float _desiredRudderAngle;

    // Simple PID for heading-based modes
    float _kP, _kI, _kD;
    float _integral, _lastErr;
    unsigned long _lastTime;
};
