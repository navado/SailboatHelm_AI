#include "RudderPositionController.h"

RudderPositionController::RudderPositionController(int pinMotorA, int pinMotorB, int analogPin)
 : _pinMotorA(pinMotorA),
   _pinMotorB(pinMotorB),
   _analogPin(analogPin),
   _currentAngle(0.0f),
   _targetAngle(0.0f),
   _kp(1.0f), _ki(0.0f), _kd(0.0f),
   _integral(0.0f), _lastError(0.0f),
   _lastUpdate(0)
{
}

bool RudderPositionController::begin()
{
    // Initialize motor pins for PWM (ESP32 LEDC as example)
    const int freq=2000, resolution=10;
    ledcSetup(0, freq, resolution);
    ledcSetup(1, freq, resolution);
    ledcAttachPin(_pinMotorA, 0);
    ledcAttachPin(_pinMotorB, 1);

    pinMode(_analogPin, INPUT);
    _lastUpdate=millis();

    Serial.println("[Rudder] Position controller started.");
    return true;
}

void RudderPositionController::setTargetAngle(float angleDeg)
{
    _targetAngle=angleDeg;
}

void RudderPositionController::update()
{
    unsigned long now=millis();
    float dt=(now-_lastUpdate)*0.001f;
    if(dt<1e-3) dt=1e-3;
    _lastUpdate=now;

    // Read current angle
    _currentAngle=readRudderSensor();

    // PID
    float error=_targetAngle-_currentAngle;
    _integral+=error*dt;
    float derivative=(error-_lastError)/dt;
    float output= _kp*error + _ki*_integral + _kd*derivative;
    _lastError=error;

    driveMotor(output);
}

float RudderPositionController::getCurrentAngle() const
{
    return _currentAngle;
}

float RudderPositionController::readRudderSensor()
{
    int raw=analogRead(_analogPin);
    float fraction=float(raw)/4095.0f;
    float angleDeg=-25.0f+(fraction*50.0f);
    return angleDeg;
}

void RudderPositionController::driveMotor(float output)
{
    float absOut=fabs(output);
    if(absOut>30.0f) absOut=30.0f; // clamp maximum

    int duty=int((absOut/30.0f)*1023);

    if(fabs(output)<0.01f) {
        // stop
        ledcWrite(0,0);
        ledcWrite(1,0);
    } else if(output>0) {
        // forward
        ledcWrite(0,duty);
        ledcWrite(1,0);
    } else {
        // reverse
        ledcWrite(0,0);
        ledcWrite(1,duty);
    }
}
