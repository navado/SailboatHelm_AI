#include "IMUFilterAndCalibration.h"
#include <cstdint>

IMUFilterAndCalibration::IMUFilterAndCalibration(IIMUProvider& imu, ITimeProvider& timeProv)
: _imu(imu)
, _time(timeProv)
, _calibrating(false)
, _pitch(0.f)
, _roll(0.f)
, _yaw(0.f)
, _lastUpdate(0)
, _axOff(0.f)
, _ayOff(0.f)
, _azOff(0.f)
{
}

void IMUFilterAndCalibration::startCalibration() {
    _calibrating = true;
}

void IMUFilterAndCalibration::doCalibrationStep() {
    if(!_calibrating) return;
    // Example: gather offsets
    _axOff = 0.f; // as needed
    _calibrating = false;
}

void IMUFilterAndCalibration::update() {
    // check if new data
    IMUData raw;
    if(!_imu.getIMUData(raw)) {
        return; // no new data
    }

    // get dt
    std::uint64_t now = _time.getMillis();
    float dt = (now - _lastUpdate)*0.001f; // ms -> sec
    if(dt < 0.0001f) dt=0.0001f;
    _lastUpdate = now;

    // apply offsets
    float ax = raw.ax - _axOff; // etc.
    // naive integration: pitch += gyroX * dt, etc.
    // or do advanced filter

    float gxDeg = raw.gx * 57.2958f; // rad/s -> deg/s
    _roll  += gxDeg * dt;  // placeholder
    // similarly for _pitch, _yaw

    // clamp or whatever
}

FilteredIMUData IMUFilterAndCalibration::getFilteredData() const {
    FilteredIMUData out;
    out.pitch = _pitch;
    out.roll  = _roll;
    out.yaw   = _yaw;
    return out;
}
