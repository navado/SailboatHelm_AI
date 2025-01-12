#include "MyIMUProvider.h"
#include <Wire.h>

// Static array from pin -> MyIMUProvider pointer
MyIMUProvider* MyIMUProvider::s_pinMap[40] = {nullptr};

MyIMUProvider::MyIMUProvider(uint8_t i2cAddr, int dataReadyPin)
: _i2cAddr(i2cAddr)
, _drPin(dataReadyPin)
, _dummyVal(0.f)
{
    // The ring buffer is default constructed
}

MyIMUProvider::~MyIMUProvider() {
    // If we want to detach the interrupt, do so
    // Also clear s_pinMap entry
    if(_drPin >= 0 && _drPin < 40) {
        s_pinMap[_drPin] = nullptr;
    }
}

bool MyIMUProvider::begin() {
    // Start I2C
    Wire.begin();
    // Possibly configure the IMU at _i2cAddr, e.g. check who_am_i, etc.

    // Setup data-ready pin
    if(_drPin < 0 || _drPin >= 40) {
        return false; // invalid pin
    }
    pinMode(_drPin, INPUT_PULLUP);
    s_pinMap[_drPin] = this; // register ourselves in the static map

    attachInterrupt(digitalPinToInterrupt(_drPin), onImuInterrupt, RISING);

    return true;
}

bool MyIMUProvider::getIMUData(IMUData& outData) {
    // pop from the ring
    return _ring.pop(outData);
}

// The static ISR callback for any pin. We can't see 'pin' directly, 
// but we know which pin triggered by the original attachInterrupt() call.
void IRAM_ATTR MyIMUProvider::onImuInterrupt() {
    // For Arduino, we can't get the pin number from within the interrupt. 
    // We simply read the status register or assume it's this pin => 
    // But we only have a single function... 
    // This is a known limitation: we can't easily detect which pin triggered 
    // if multiple pins share the same function. 
    // One approach is to rely on the fact that only one pin triggers at a time 
    // or do digitalRead, etc. If you want multiple pins, you'd do multiple attachInterrupt calls 
    // each referencing a different static function. 
    //
    // For demonstration, let's do a naive approach that tries all known pins:
    for(int p=0; p<40; p++){
        if(s_pinMap[p]) {
            // check if that pin is actually LOW => means triggered?
            if(digitalRead(p)==LOW){
                s_pinMap[p]->readSensorInISR();
            }
        }
    }
}

void MyIMUProvider::readSensorInISR() {
    // In real code: read from sensor registers quickly
    IMUData reading;
    reading.ax = 0.f;
    reading.ay = 0.f;
    reading.az = 9.81f;
    reading.gx = _dummyVal;
    reading.gy = 0.f;
    reading.gz = 0.f;
    reading.mx = 0.f;
    reading.my = 0.f;
    reading.mz = 0.f;

    _dummyVal += 0.1f;

    // push to ring
    _ring.push(reading);
}
