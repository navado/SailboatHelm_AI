#include "MyIMUProvider.h"
#include <Wire.h>

static const int PIN_IMU_DATA_READY = 8;

MyIMUProvider* MyIMUProvider::instance = nullptr;

MyIMUProvider::MyIMUProvider()
: _dummyVal(0.f)
{
    // ring buffer default constructed
}

MyIMUProvider::~MyIMUProvider() {
    if(instance == this) {
        instance = nullptr;
    }
}

bool MyIMUProvider::begin() {
    Wire.begin();
    pinMode(PIN_IMU_DATA_READY, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_IMU_DATA_READY),
                    onImuDataReady,
                    RISING);
    instance = this;
    return true;
}

bool MyIMUProvider::getIMUData(IMUData& outData) {
    // ring pop
    return _ring.pop(outData);
}

void IRAM_ATTR MyIMUProvider::onImuDataReady() {
    if(!instance) return;
    instance->readSensorInISR();
}

void MyIMUProvider::readSensorInISR() {
    // Real code: read sensor registers quickly, or set a flag
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

    _dummyVal += 0.1f; // example

    // push to ring
    // if the ring is full, push() returns false => we skip
    _ring.push(reading);
}
