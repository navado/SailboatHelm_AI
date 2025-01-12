#pragma once

/**
 * Abstract interface that provides IMU data:
 * e.g. accelerometer, gyro, mag readings.
 * The actual I2C or hardware code is done in platform-specific implementation.
 */

struct IMUData {
    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;

    // Copy constructor
    IMUData(const IMUData& other)
        : ax(other.ax), ay(other.ay), az(other.az),
          gx(other.gx), gy(other.gy), gz(other.gz),
          mx(other.mx), my(other.my), mz(other.mz) {}

    // Default constructor
    IMUData() : ax(0), ay(0), az(0), gx(0), gy(0), gz(0), mx(0), my(0), mz(0) {}
};

class IIMUProvider {
public:
    virtual ~IIMUProvider() = default;

    /**
     * Poll or fetch the latest sensor data. 
     * Return 'true' if new data was successfully retrieved.
     */
    virtual bool getIMUData(IMUData& outData) = 0;
};
