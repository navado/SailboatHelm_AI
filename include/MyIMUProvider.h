#pragma once

#include "IIMUProvider.h"
#include "RingBuffer.h" // our new ring buffer

/**
 * MyIMUProvider uses our ring buffer to store IMUData
 * from an interrupt (data-ready pin).
 */
class MyIMUProvider : public IIMUProvider {
public:
    MyIMUProvider();
    ~MyIMUProvider();

    bool begin();

    // from IIMUProvider
    bool getIMUData(IMUData& outData);
    static void onImuDataReady();

    void readSensorInISR();
private:

    // Our ring buffer for IMUData
    RingBuffer<IMUData, 16> _ring;

    // Example usage
    float _dummyVal;

    static MyIMUProvider* instance;
};
