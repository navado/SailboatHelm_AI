#include "AutoSteeringController.h"

AutoSteeringController::AutoSteeringController()
 : _mode(AutoSteeringMode::OFF),
   _desiredHeading(0.0f),
   _desiredCourse(0.0f),
   _desiredWindAngle(0.0f),
   _lockAngle(0.0f),
   _desiredRudderAngle(0.0f),
   _kP(1.0f), _kI(0.0f), _kD(0.0f),
   _integral(0.0f), _lastErr(0.0f),
   _lastTime(0)
{
    _env.heading=0.0f;
    _env.course=0.0f;
    _env.windDirection=0.0f;
}

void AutoSteeringController::setMode(AutoSteeringMode mode, float param)
{
    _mode=mode;
    // reset integrals
    _integral=0.0f;
    _lastErr=0.0f;

    switch(_mode) {
    case AutoSteeringMode::OFF:
        break;

    case AutoSteeringMode::TRACK_HEADING:
        _desiredHeading=param;
        break;

    case AutoSteeringMode::TRACK_COURSE:
        _desiredCourse=param;
        break;

    case AutoSteeringMode::TRACK_WIND_ANGLE:
        _desiredWindAngle=param;
        break;

    case AutoSteeringMode::RUDDER_LOCK:
    {
        // Implement safe limits, e.g. [-25..25]
        if(param>25.0f) param=25.0f;
        if(param<-25.0f) param=-25.0f;
        // We'll store it in _lockAngle
        _lockAngle=param;
        Serial.printf("[AutoSteer] Entering RUDDER_LOCK at %.2f deg\n", _lockAngle);
    }
    break;

    default:
        // calibrations...
        break;
    }
}

void AutoSteeringController::updateEnvironmentData(const EnvironmentData& env)
{
    _env=env;
}

void AutoSteeringController::update()
{
    computeSteering();
}

void AutoSteeringController::computeSteering()
{
    unsigned long now=millis();
    float dt=(now-_lastTime)*0.001f;
    if(dt<1e-3) dt=1e-3;
    _lastTime=now;

    float error=0.0f;
    switch(_mode) {
    case AutoSteeringMode::OFF:
        _desiredRudderAngle=0.0f;
        return;

    case AutoSteeringMode::TRACK_HEADING:
        error=_desiredHeading-_env.heading;
        break;

    case AutoSteeringMode::TRACK_COURSE:
        error=_desiredCourse-_env.course;
        break;

    case AutoSteeringMode::TRACK_WIND_ANGLE:
        error=_desiredWindAngle-_env.windDirection;
        break;

    case AutoSteeringMode::RUDDER_LOCK:
        // Keep the rudder locked at _lockAngle
        _desiredRudderAngle=_lockAngle;
        return;

    default:
        return;
    }

    // If we get here, we are in a heading-based mode => simple PID
    _integral+=error*dt;
    float derivative=(error-_lastErr)/dt;
    float output= _kP*error + _kI*_integral + _kD*derivative;
    _lastErr=error;

    // clamp
    if(output>25.0f) output=25.0f;
    if(output<-25.0f) output=-25.0f;

    _desiredRudderAngle=output;
}

float AutoSteeringController::getDesiredRudderAngle() const
{
    return _desiredRudderAngle;
}
