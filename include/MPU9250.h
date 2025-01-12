#pragma once

#include <Arduino.h>
#include <Wire.h>

// In your minimal "MPU9250.h"
enum class MPU9250AccelRange {
    ACCEL_RANGE_2G  = 0,
    ACCEL_RANGE_4G  = 1,
    ACCEL_RANGE_8G  = 2,
    ACCEL_RANGE_16G = 3
};

enum class MPU9250GyroRange {
    GYRO_RANGE_250DPS  = 0,
    GYRO_RANGE_500DPS  = 1,
    GYRO_RANGE_1000DPS = 2,
    GYRO_RANGE_2000DPS = 3
};

// You might define a method or enum for DLPF:
static const uint8_t DLPF_BANDWIDTH_20HZ = 4;


/**
 * A minimal class to interface with the MPU9250 via I2C,
 * read raw data, and provide converted values.
 */
class MPU9250 {
public:
    /**
     * Constructor
     * @param wire   The I2C interface (e.g. Wire)
     * @param address The I2C address (default 0x68 for MPU9250)
     */
    MPU9250(TwoWire& wire, uint8_t address=0x68);

    /**
     * Initialize the sensor: check WHO_AM_I, configure accel/gyro,
     * enable I2C master for mag, etc.
     * @return 0 on success, negative on fail
     */
    int begin();

    /**
     * Set accelerometer range (2G, 4G, 8G, 16G)
     */
    void setAccelRange(MPU9250AccelRange range);

    /**
     * Set gyroscope range (250, 500, 1000, 2000 dps)
     */
    void setGyroRange(MPU9250GyroRange range);

    /**
     * Read the sensor data (accel, gyro, mag) from the device.
     * Store in internal buffers. 
     * Typically called in your loop or IMU update routine.
     */
    void readSensor();

    /**
     * Get the accelerometer in m/s^2 for X, Y, Z
     */
    float getAccelX_mSs() const { return _accelX; }
    float getAccelY_mSs() const { return _accelY; }
    float getAccelZ_mSs() const { return _accelZ; }

    /**
     * Get the gyroscope in rad/s for X, Y, Z
     */
    float getGyroX_rads() const { return _gyroX; }
    float getGyroY_rads() const { return _gyroY; }
    float getGyroZ_rads() const { return _gyroZ; }

    /**
     * Get the magnetometer in microtesla (uT) for X, Y, Z
     */
    float getMagX_uT() const { return _magX; }
    float getMagY_uT() const { return _magY; }
    float getMagZ_uT() const { return _magZ; }

    /**
     * (Optional) set digital low-pass filter bandwidth or sample rate
     * (Not fully implemented in this minimal example.)
     */
    void setDlpfBandwidth(uint8_t bandwidth) { _dlpfMode = bandwidth; }
    void setSrd(uint8_t srd) { _srd = srd; }

private:
    /**
     * Low-level I2C read/write helpers
     */
    uint8_t readByte(uint8_t reg);
    void    readBytes(uint8_t reg, uint8_t count, uint8_t * dest);
    void    writeByte(uint8_t reg, uint8_t data);

    /**
     * Setup magnetometer (AK8963) in pass-through / I2C master mode
     */
    void initMag();

    /**
     * Read raw magnetometer data from AK8963.
     */
    void readMagData();

private:
    TwoWire&  _wire;
    uint8_t   _address;

    // range settings
    MPU9250AccelRange _accelRange;
    MPU9250GyroRange  _gyroRange;
    uint8_t           _dlpfMode;
    uint8_t           _srd;

    // scaled data
    float _accelX, _accelY, _accelZ;
    float _gyroX,  _gyroY,  _gyroZ;
    float _magX,   _magY,   _magZ;

    // conversion factors
    float _accelScale;
    float _gyroScale;

    // magnetometer I2C address
    static const uint8_t AK8963_ADDRESS     = 0x0C;
};
