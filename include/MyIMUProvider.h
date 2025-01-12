#pragma once

#include <Arduino.h>
#include "IIMUProvider.h"
#include "RingBuffer.h"

/**
 * A platform-specific class that implements IIMUProvider,
 * reading IMU data in an ISR when the IMU data-ready pin goes HIGH.
 * 
 * The constructor receives:
 *  - i2cAddr:   the I2C address of the IMU
 *  - dataReadyPin: the pin where the IMU DRDY is connected
 */
class MyIMUProvider : public IIMUProvider {
public:
    // We let the user pass i2c address and data-ready pin
    MyIMUProvider(uint8_t i2cAddr, int dataReadyPin);

    ~MyIMUProvider() override;

    // Initialize hardware (I2C, attach interrupt, etc.)
    bool begin();

    // From IIMUProvider: get the latest IMU sample (if available)
    bool getIMUData(IMUData& outData) override;

    // We'll declare a static function for the ISR, 
    // but we DO NOT store a global 'this' pointer => we store in a map.
    static void IRAM_ATTR onImuInterrupt();

private:
    // The function that actually reads sensor in the ISR
    void readSensorInISR();

    // Our ring buffer
    static constexpr int RB_CAPACITY = 16;
    RingBuffer<IMUData, RB_CAPACITY> _ring;

    uint8_t  _i2cAddr;       // e.g. 0x68 or 0x69
    int      _drPin;         // data-ready pin
    float    _dummyVal;      // example usage

    // To handle multiple objects, we store a static map from pin -> object
    // We'll assume a small maximum pin number. Or we can use a std::unordered_map if platform allows.
    static MyIMUProvider* s_pinMap[40]; // or 64, or 256, etc.

    // We'll store an index for ourselves, or we can store the pin
    // and s_pinMap[drPin] = this
};
