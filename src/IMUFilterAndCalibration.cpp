#include "IMUFilterAndCalibration.h"

IMUFilterAndCalibration::IMUFilterAndCalibration()
 : _mpu(Wire, 0x68),
   _intPin(-1),
   _pitch(0.0f),
   _roll(0.0f),
   _yaw(0.0f),
   _inCalibration(false)
{
    // Default offsets = 0
    _calData = {0,0,0, 0,0,0, 0,0,0};
}

bool IMUFilterAndCalibration::begin(int sdaPin, int sclPin, int intPin)
{
    _intPin = intPin;

    // Init filesystem
    initLittleFS();
    // Load existing calibration if present
    loadCalibration();

    // I2C init
    Wire.begin(sdaPin, sclPin);
    Wire.setClock(400000);

    // Init MPU9250
    int status = _mpu.begin();
    if (status < 0) {
        Serial.println("[IMU] MPU9250 init failed!");
        return false;
    }
    _mpu.setAccelRange(MPU9250AccelRange::ACCEL_RANGE_2G);
    _mpu.setGyroRange(MPU9250GyroRange::GYRO_RANGE_250DPS);
    _mpu.setDlpfBandwidth(DLPF_BANDWIDTH_20HZ);
    _mpu.setSrd(4); // ~200Hz

    pinMode(_intPin, INPUT_PULLUP);
    // Optionally attachInterrupt if needed for data-ready

    Serial.println("[IMU] begin OK");
    return true;
}

bool IMUFilterAndCalibration::initLittleFS()
{
    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS mount failed / formatted!");
        return false;
    }
    return true;
}

bool IMUFilterAndCalibration::loadCalibration()
{
    File f = LittleFS.open("/imu_cal.json", "r");
    if (!f) {
        Serial.println("[IMU] No calibration file found.");
        return false;
    }

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err) {
        Serial.print("[IMU] JSON parse error: ");
        Serial.println(err.c_str());
        return false;
    }

    _calData.accelOffsetX = doc["ax"] | 0.0f;
    _calData.accelOffsetY = doc["ay"] | 0.0f;
    _calData.accelOffsetZ = doc["az"] | 0.0f;
    _calData.gyroOffsetX  = doc["gx"] | 0.0f;
    _calData.gyroOffsetY  = doc["gy"] | 0.0f;
    _calData.gyroOffsetZ  = doc["gz"] | 0.0f;
    _calData.magOffsetX   = doc["mx"] | 0.0f;
    _calData.magOffsetY   = doc["my"] | 0.0f;
    _calData.magOffsetZ   = doc["mz"] | 0.0f;

    Serial.println("[IMU] Calibration loaded from /imu_cal.json");
    return true;
}

bool IMUFilterAndCalibration::saveCalibration()
{
    StaticJsonDocument<512> doc;
    doc["ax"] = _calData.accelOffsetX;
    doc["ay"] = _calData.accelOffsetY;
    doc["az"] = _calData.accelOffsetZ;
    doc["gx"] = _calData.gyroOffsetX;
    doc["gy"] = _calData.gyroOffsetY;
    doc["gz"] = _calData.gyroOffsetZ;
    doc["mx"] = _calData.magOffsetX;
    doc["my"] = _calData.magOffsetY;
    doc["mz"] = _calData.magOffsetZ;

    File f = LittleFS.open("/imu_cal.json", "w");
    if (!f) {
        Serial.println("[IMU] Could not open /imu_cal.json for writing");
        return false;
    }
    serializeJson(doc, f);
    f.close();

    Serial.println("[IMU] Calibration saved to /imu_cal.json");
    return true;
}

void IMUFilterAndCalibration::startCalibration()
{
    Serial.println("[IMU] Start calibration procedure...");
    _inCalibration = true;
}

void IMUFilterAndCalibration::doCalibrationStep()
{
    if (!_inCalibration) return;
    // Gather offsets, measure sensor data...
    // For demonstration, do trivial:
    _inCalibration = false;
    saveCalibration();
    Serial.println("[IMU] Calibration done.");
}

void IMUFilterAndCalibration::update()
{
    // read data
    _mpu.readSensor();
    float ax = _mpu.getAccelX_mSs() - _calData.accelOffsetX;
    float ay = _mpu.getAccelY_mSs() - _calData.accelOffsetY;
    float az = _mpu.getAccelZ_mSs() - _calData.accelOffsetZ;

    float gx = _mpu.getGyroX_rads() - _calData.gyroOffsetX;
    float gy = _mpu.getGyroY_rads() - _calData.gyroOffsetY;
    float gz = _mpu.getGyroZ_rads() - _calData.gyroOffsetZ;

    float mx = _mpu.getMagX_uT()  - _calData.magOffsetX;
    float my = _mpu.getMagY_uT()  - _calData.magOffsetY;
    float mz = _mpu.getMagZ_uT()  - _calData.magOffsetZ;

    // Very naive integration filter
    static unsigned long lastT=millis();
    unsigned long nowT=millis();
    float dt=(nowT-lastT)*0.001f;
    if(dt<1e-3) dt=1e-3;
    lastT=nowT;

    float gxDeg = gx*(180.0f/PI);
    float gyDeg = gy*(180.0f/PI);
    float gzDeg = gz*(180.0f/PI);

    _roll  += gxDeg*dt;
    _pitch += gyDeg*dt;
    _yaw   += gzDeg*dt;

    // keep in range or do advanced fusion...
}

FilteredIMUData IMUFilterAndCalibration::getFilteredData() const
{
    FilteredIMUData out;
    out.pitch = _pitch;
    out.roll  = _roll;
    out.yaw   = _yaw;
    return out;
}
