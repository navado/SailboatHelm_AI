#pragma once

#include <Arduino.h>

/**
 * Class that runs a local PID to keep the rudder at a desired angle,
 * using an analog pot for feedback, and two PWM pins for forward/reverse.
 */
class RudderPositionController {
public:
    RudderPositionController(int pinMotorA, int pinMotorB, int analogPin);

    // Setup PWM for the motor pins, etc.
    bool begin();

    // Update closed-loop control. Call ~50-100Hz.
    void update();

    // External code sets target angle (in degrees).
    void setTargetAngle(float angleDeg);

    // Read the actual angle from pot (used internally).
    float getCurrentAngle() const;

private:
    // Reads analog pot, converts to [-25..25] deg or whatever range.
    float readRudderSensor();

    // Drive H-bridge with sign for direction.
    void driveMotor(float output);

private:
    int _pinMotorA;
    int _pinMotorB;
    int _analogPin;

    float _currentAngle;
    float _targetAngle;

    // PID terms
    float _kp, _ki, _kd;
    float _integral, _lastError;

    unsigned long _lastUpdate;
};
