#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// Example IMU library (MPU9250). Adjust if using a different sensor:
#include <MPU9250.h>

/**
 * Data structure for storing IMU calibration offsets, etc.
 */
struct IMUCalibrationData {
    float accelOffsetX;
    float accelOffsetY;
    float accelOffsetZ;
    float gyroOffsetX;
    float gyroOffsetY;
    float gyroOffsetZ;
    float magOffsetX;
    float magOffsetY;
    float magOffsetZ;
};

/**
 * Filtered IMU data structure to hold final pitch/roll/yaw.
 */
struct FilteredIMUData {
    float pitch;
    float roll;
    float yaw;
};

/**
 * Class that handles IMU reading, filtering, and calibration data.
 * Uses LittleFS to save/read a JSON file with offsets.
 */
class IMUFilterAndCalibration {
public:
    IMUFilterAndCalibration();

    // Initialize IMU, load calibration from file, etc.
    bool begin(int sdaPin, int sclPin, int intPin);

    // Attempt to load calibration offsets from /imu_cal.json
    bool loadCalibration();

    // Save offsets to /imu_cal.json
    bool saveCalibration();

    // Start a calibration procedure
    void startCalibration();

    // Perform a step of calibration
    void doCalibrationStep();

    // Called periodically (or when data-ready) to read + filter IMU
    void update();

    // Get the final, filtered orientation
    FilteredIMUData getFilteredData() const;

private:
    bool initLittleFS();

private:
    MPU9250 _mpu;
    int     _intPin;

    IMUCalibrationData _calData;

    float _pitch;
    float _roll;
    float _yaw;

    bool  _inCalibration;
};
