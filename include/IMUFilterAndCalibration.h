#pragma once
#include "IIMUProvider.h"
#include "ITimeProvider.h"

/**
 * Example structure for storing pitch/roll/yaw
 * from a naive or advanced filter.
 */
struct FilteredIMUData {
    float pitch;
    float roll;
    float yaw;
};

class IMUFilterAndCalibration {
public:
    IMUFilterAndCalibration(IIMUProvider& imu, ITimeProvider& timeProv);

    // Start or do calibration
    void startCalibration();
    void doCalibrationStep();

    // Called periodically
    void update();

    // Get the fused orientation
    FilteredIMUData getFilteredData() const;

private:
    IIMUProvider&      _imu;
    ITimeProvider&     _time;
    bool               _calibrating;
    float              _pitch, _roll, _yaw;
    std::uint64_t      _lastUpdate;
    // Example offsets
    float _axOff, _ayOff, _azOff;
    // etc.
};
