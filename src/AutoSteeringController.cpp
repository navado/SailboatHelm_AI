#include "AutoSteeringController.h"
#include <cmath>

AutoSteeringController::AutoSteeringController()
: _mode(AutoSteeringMode::OFF)
, _desiredHeading(0.f)
, _desiredCourse(0.f)
, _desiredWindAngle(0.f)
, _rudderAngle(0.f)
, _kP(1.f)
, _kI(0.f)
, _kD(0.f)
, _integral(0.f)
, _lastError(0.f)
{
}

void AutoSteeringController::setMode(AutoSteeringMode mode, float param) {
    _mode = mode;
    _integral = 0.f;
    _lastError= 0.f;
    switch(mode) {
        case AutoSteeringMode::OFF:
            break;
        case AutoSteeringMode::TRACK_HEADING:
            _desiredHeading = param;
            break;
        case AutoSteeringMode::TRACK_COURSE:
            _desiredCourse = param;
            break;
        case AutoSteeringMode::TRACK_WIND_ANGLE:
            _desiredWindAngle = param;
            break;
    }
}

void AutoSteeringController::update(float dt) {
    computeSteering(dt);
}

float AutoSteeringController::getRudderAngle() const {
    return _rudderAngle;
}

void AutoSteeringController::computeSteering(float dt) {
    if(_mode == AutoSteeringMode::OFF) {
        _rudderAngle = 0.f;
        return;
    }

    // placeholder for heading logic
    float error = 0.f;
    switch(_mode) {
        case AutoSteeringMode::TRACK_HEADING:
            // Suppose we have an external function that gives us the actual heading?
            // We might pass it in. For demonstration, assume "actualHeading=somewhere"
            // We'll just do a placeholder:
            error = _desiredHeading - 0.f; // TOTALLY FAKE
            break;
        case AutoSteeringMode::TRACK_COURSE:
            error = _desiredCourse - 0.f; 
            break;
        case AutoSteeringMode::TRACK_WIND_ANGLE:
            error = _desiredWindAngle - 0.f; 
            break;
        default:
            _rudderAngle=0.f;
            return;
    }

    _integral += error*dt;
    float derivative = (error - _lastError)/dt;
    float output = _kP*error + _kI*_integral + _kD*derivative;
    _lastError=error;

    // clamp e.g. [-30..30] deg
    if(output>30.f) output=30.f;
    if(output<-30.f)output=-30.f;
    _rudderAngle = output;
}
