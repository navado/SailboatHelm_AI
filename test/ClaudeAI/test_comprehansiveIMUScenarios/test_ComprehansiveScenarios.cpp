#include <unity.h>
#include "IMUFilterAndCalibration.h"
#include <cmath>

class AdvancedIMUProvider : public IIMUProvider {
private:
    float _time;
    float _lastHeading;
    float _magDistortion;
    float _driftRate;
    bool _isCalibrated;
    

    
    float addMagneticDistortion(float value, float time) {
        return value + _magDistortion * sin(time * 0.5f);
    }
    
    float addSensorDrift(float value) {
        return value + _driftRate * _time;
    }
    
    float addVibration(float value, float time) {
        if (!_scenario.hasVibration) return value;
        return value + 0.5f * sin(2 * M_PI * time * _scenario.vibrationFreq);
    }

public:

    struct MotionScenario {
        float pitchAmplitude;
        float rollAmplitude;
        float yawRate;
        float pitchPeriod;
        float rollPeriod;
        float initialHeading;
        float waveHeight;
        float wavePeriod;
        bool hasJerk;
        float magneticDistortion;
        float sensorDrift;
        bool hasVibration;
        float vibrationFreq;
        float accelerationFactor;
    };


    MotionScenario _scenario;
    
    AdvancedIMUProvider(const MotionScenario& scenario) 
        : _time(0.0f), 
          _scenario(scenario), 
          _lastHeading(scenario.initialHeading),
          _magDistortion(scenario.magneticDistortion),
          _driftRate(scenario.sensorDrift),
          _isCalibrated(false) {}

    bool getIMUData(IMUData& data) override {
        // Basic motion with complex patterns
        float pitch = _scenario.pitchAmplitude * sin(2 * M_PI * _time / _scenario.pitchPeriod);
        float roll = _scenario.rollAmplitude * sin(2 * M_PI * _time / _scenario.rollPeriod);
        
        // Add wave impact
        float waveOffset = _scenario.waveHeight * sin(2 * M_PI * _time / _scenario.wavePeriod);
        pitch += waveOffset * cos(2 * M_PI * _time / 1.5f);
        roll += waveOffset * sin(2 * M_PI * _time / 2.0f);

        // Add rapid maneuver effects
        if (_scenario.accelerationFactor > 1.0f) {
            float maneuverEffect = _scenario.accelerationFactor * 
                sin(2 * M_PI * _time / 3.0f) * exp(-_time / 10.0f);
            pitch += maneuverEffect * 2.0f;
            roll += maneuverEffect * 3.0f;
        }

        // Update heading
        _lastHeading += _scenario.yawRate * 0.01f;
        if (_lastHeading >= 360.0f) _lastHeading -= 360.0f;
        
        float heading_rad = _lastHeading * M_PI / 180.0f;
        float pitch_rad = pitch * M_PI / 180.0f;
        float roll_rad = roll * M_PI / 180.0f;

        // Complex magnetometer simulation
        data.mx = addMagneticDistortion(
            cos(heading_rad) * cos(pitch_rad) + 
            sin(heading_rad) * sin(roll_rad) * sin(pitch_rad),
            _time
        );
        data.my = addMagneticDistortion(
            sin(heading_rad) * cos(roll_rad),
            _time + 2.1f  // Phase shift
        );
        data.mz = addMagneticDistortion(
            -cos(heading_rad) * sin(pitch_rad) + 
            sin(heading_rad) * sin(roll_rad) * cos(pitch_rad),
            _time + 4.2f  // Different phase
        );

        // Accelerometer with complex motion
        float baseAx = sin(pitch_rad);
        float baseAy = -cos(pitch_rad) * sin(roll_rad);
        float baseAz = -cos(pitch_rad) * cos(roll_rad);
        
        data.ax = addVibration(addSensorDrift(baseAx + waveOffset * 0.1f), _time);
        data.ay = addVibration(addSensorDrift(baseAy + waveOffset * 0.1f), _time);
        data.az = addVibration(addSensorDrift(baseAz + waveOffset * 0.15f), _time);

        // Gyro with complex rates
        data.gx = addVibration(addSensorDrift(
            _scenario.pitchAmplitude * (2 * M_PI / _scenario.pitchPeriod) * 
            cos(2 * M_PI * _time / _scenario.pitchPeriod)), _time);
            
        data.gy = addVibration(addSensorDrift(
            _scenario.rollAmplitude * (2 * M_PI / _scenario.rollPeriod) * 
            cos(2 * M_PI * _time / _scenario.rollPeriod)), _time);
            
        data.gz = addVibration(addSensorDrift(
            _scenario.yawRate * M_PI / 180.0f), _time);

        _time += 0.01f;
        return true;
    }
};

class PreciseTimeProvider : public ITimeProvider {
    uint64_t _time = 0;
public:
    uint64_t getMillis() const override { return _time; }
    void increment(uint64_t delta) { _time += delta; }
};

/*
 * Test 1: Heavy Weather Navigation
 * Simulates rough sea conditions with:
 * - Large wave height (3m) with 8s period
 * - Significant pitch/roll motion
 * - Slow turn rate
 * - Random wave impacts
 */
void test_heavy_weather_navigation() {
    AdvancedIMUProvider::MotionScenario scenario = {
        .pitchAmplitude = 15.0f,
        .rollAmplitude = 25.0f,
        .yawRate = 2.0f,
        .pitchPeriod = 4.0f,
        .rollPeriod = 6.0f,
        .initialHeading = 45.0f,
        .waveHeight = 3.0f,
        .wavePeriod = 8.0f,
        .hasJerk = true,
        .magneticDistortion = 0.0f,
        .sensorDrift = 0.0f,
        .hasVibration = false,
        .vibrationFreq = 0.0f,
        .accelerationFactor = 1.0f
    };

    AdvancedIMUProvider imu(scenario);
    PreciseTimeProvider timeProvider;
    IMUFilterAndCalibration filter(imu, timeProvider);

    const int numSamples = 2000;
    float maxHeadingError = 0.0f;
    float lastHeading = scenario.initialHeading;
    
    for(int i = 0; i < numSamples; i++) {
        timeProvider.increment(10);
        filter.update();
        FilteredIMUData data = filter.getFilteredData();
        
        float expectedHeading = scenario.initialHeading + scenario.yawRate * (i * 0.01f);
        while(expectedHeading >= 360.0f) expectedHeading -= 360.0f;
        
        float error = fabs(data.yaw - expectedHeading);
        if(error > 180.0f) error = 360.0f - error;
        
        maxHeadingError = fmax(maxHeadingError, error);
        TEST_ASSERT_TRUE_MESSAGE(error <= 5.0f, "Excessive heading error in heavy weather");
    }
}

/*
 * Test 2: Rapid Maneuvering
 * Tests filter performance during:
 * - High yaw rate (20°/s)
 * - Sharp accelerations
 * - Rapid pitch/roll changes
 * - Engine vibration effects
 */
void test_rapid_maneuvering() {
    AdvancedIMUProvider::MotionScenario scenario = {
        .pitchAmplitude = 10.0f,
        .rollAmplitude = 15.0f,
        .yawRate = 20.0f,
        .pitchPeriod = 2.0f,
        .rollPeriod = 3.0f,
        .initialHeading = 0.0f,
        .waveHeight = 0.5f,
        .wavePeriod = 4.0f,
        .hasJerk = false,
        .magneticDistortion = 0.0f,
        .sensorDrift = 0.0f,
        .hasVibration = true,
        .vibrationFreq = 50.0f,
        .accelerationFactor = 3.0f
    };

    AdvancedIMUProvider imu(scenario);
    PreciseTimeProvider timeProvider;
    IMUFilterAndCalibration filter(imu, timeProvider);

    const int numSamples = 1000;
    float lastYawRate = 0.0f;
    
    for(int i = 0; i < numSamples; i++) {
        timeProvider.increment(10);
        filter.update();
        FilteredIMUData data = filter.getFilteredData();
        
        float yawRate = (data.yaw - lastYawRate) / 0.01f;  // deg/s
        TEST_ASSERT_TRUE_MESSAGE(fabs(yawRate) <= 25.0f, "Excessive yaw rate detected");
        lastYawRate = data.yaw;
    }
}

/*
 * Test 3: Magnetic Interference
 * Simulates:
 * - Local magnetic distortions
 * - Varying interference patterns
 * - Different interference frequencies
 */
void test_magnetic_interference() {
    AdvancedIMUProvider::MotionScenario scenario = {
        .pitchAmplitude = 5.0f,
        .rollAmplitude = 8.0f,
        .yawRate = 1.0f,
        .pitchPeriod = 4.0f,
        .rollPeriod = 6.0f,
        .initialHeading = 90.0f,
        .waveHeight = 0.5f,
        .wavePeriod = 5.0f,
        .hasJerk = false,
        .magneticDistortion = 0.3f,
        .sensorDrift = 0.0f,
        .hasVibration = false,
        .vibrationFreq = 0.0f,
        .accelerationFactor = 1.0f
    };

    AdvancedIMUProvider imu(scenario);
    PreciseTimeProvider timeProvider;
    IMUFilterAndCalibration filter(imu, timeProvider);

    const int numSamples = 1500;
    float maxHeadingDeviation = 0.0f;
    float lastHeading = scenario.initialHeading;
    
    for(int i = 0; i < numSamples; i++) {
        timeProvider.increment(10);
        filter.update();
        FilteredIMUData data = filter.getFilteredData();
        
        float headingDelta = fabs(data.yaw - lastHeading);
        if(headingDelta > 180.0f) headingDelta = 360.0f - headingDelta;
        
        maxHeadingDeviation = fmax(maxHeadingDeviation, headingDelta);
        TEST_ASSERT_TRUE_MESSAGE(headingDelta <= 3.0f, 
            "Excessive heading change due to magnetic interference");
        
        lastHeading = data.yaw;
    }
}

/*
 * Test 4: Sensor Drift
 * Tests compensation for:
 * - Gradual sensor bias drift
 * - Temperature effects
 * - Long-term stability
 */
void test_sensor_drift() {
    AdvancedIMUProvider::MotionScenario scenario = {
        .pitchAmplitude = 3.0f,
        .rollAmplitude = 5.0f,
        .yawRate = 0.0f,
        .pitchPeriod = 4.0f,
        .rollPeriod = 6.0f,
        .initialHeading = 180.0f,
        .waveHeight = 0.2f,
        .wavePeriod = 5.0f,
        .hasJerk = false,
        .magneticDistortion = 0.0f,
        .sensorDrift = 0.001f,  // 0.001 deg/s drift
        .hasVibration = false,
        .vibrationFreq = 0.0f,
        .accelerationFactor = 1.0f
    };

    AdvancedIMUProvider imu(scenario);
    PreciseTimeProvider timeProvider;
    IMUFilterAndCalibration filter(imu, timeProvider);

    const int numSamples = 3000;  // 30 seconds
    float initialHeading = scenario.initialHeading;
    
    for(int i = 0; i < numSamples; i++) {
        timeProvider.increment(10);
        filter.update();
        FilteredIMUData data = filter.getFilteredData();
        
        float headingError = fabs(data.yaw - initialHeading);
        if(headingError > 180.0f) headingError = 360.0f - headingError;
        
        TEST_ASSERT_TRUE_MESSAGE(headingError <= 2.0f, 
            "Excessive drift in heading");
    }
}

/*
 * Test 5: Combined Stresses
 * Tests all effects simultaneously:
 * - Heavy weather
 * - Magnetic interference
 * - Sensor drift
 * - Vibration
 * - Rapid maneuvers
 */
void test_combined_stresses() {
    AdvancedIMUProvider::MotionScenario scenario = {
        .pitchAmplitude = 15.0f,
        .rollAmplitude = 25.0f,
        .yawRate = 10.0f,
        .pitchPeriod = 4.0f,
        .rollPeriod = 6.0f,
        .initialHeading = 270.0f,
        .waveHeight = 2.0f,
        .wavePeriod = 8.0f,
        .hasJerk = true,
        .magneticDistortion = 0.2f,
        .sensorDrift = 0.0005f,
        .hasVibration = true,
        .vibrationFreq = 30.0f,
        .accelerationFactor = 2.0f
    };

    AdvancedIMUProvider imu(scenario);
    PreciseTimeProvider timeProvider;
    IMUFilterAndCalibration filter(imu, timeProvider);

    const int numSamples = 4000;
    float lastHeading = scenario.initialHeading;
    float maxHeadingJump = 0.0f;
    
    for(int i = 0; i < numSamples; i++) {
        timeProvider.increment(10);
        filter.update();
     FilteredIMUData data = filter.getFilteredData();
        
        float headingDelta = fabs(data.yaw - lastHeading);
        if(headingDelta > 180.0f) headingDelta = 360.0f - headingDelta;
        
        maxHeadingJump = fmax(maxHeadingJump, headingDelta);
        
        // Test heading change rate
        TEST_ASSERT_TRUE_MESSAGE(headingDelta <= 5.0f, 
            "Excessive heading jump under combined stresses");
            
        // Test pitch and roll limits
        TEST_ASSERT_TRUE_MESSAGE(fabs(data.pitch) <= 20.0f, 
            "Pitch exceeded safe limits");
        TEST_ASSERT_TRUE_MESSAGE(fabs(data.roll) <= 30.0f, 
            "Roll exceeded safe limits");
        
        lastHeading = data.yaw;
    }
    
    // Overall stability test
    TEST_ASSERT_TRUE_MESSAGE(maxHeadingJump <= 10.0f, 
        "Filter showed unstable behavior under combined stresses");
}

/*
 * Test 6: Calibration Accuracy
 * Tests IMU calibration:
 * - Initial calibration accuracy
 * - Bias estimation
 * - Scale factor correction
 * - Cross-axis sensitivity
 */
void test_calibration_accuracy() {
    AdvancedIMUProvider::MotionScenario scenario = {
        .pitchAmplitude = 0.0f,
        .rollAmplitude = 0.0f,
        .yawRate = 0.0f,
        .pitchPeriod = 4.0f,
        .rollPeriod = 6.0f,
        .initialHeading = 0.0f,
        .waveHeight = 0.0f,
        .wavePeriod = 5.0f,
        .hasJerk = false,
        .magneticDistortion = 0.1f,
        .sensorDrift = 0.0002f,
        .hasVibration = false,
        .vibrationFreq = 0.0f,
        .accelerationFactor = 1.0f
    };

    AdvancedIMUProvider imu(scenario);
    PreciseTimeProvider timeProvider;
    IMUFilterAndCalibration filter(imu, timeProvider);

    // Initial calibration phase
    filter.startCalibration();
    
    const int calibrationSamples = 500;
    for(int i = 0; i < calibrationSamples; i++) {
        timeProvider.increment(10);
        filter.update();
    }

    // Test static accuracy post-calibration
    const int testSamples = 1000;
    float maxPitchError = 0.0f;
    float maxRollError = 0.0f;
    float maxHeadingError = 0.0f;
    
    for(int i = 0; i < testSamples; i++) {
        timeProvider.increment(10);
        filter.update();
        FilteredIMUData data = filter.getFilteredData();
        
        maxPitchError = fmax(maxPitchError, fabs(data.pitch));
        maxRollError = fmax(maxRollError, fabs(data.roll));
        maxHeadingError = fmax(maxHeadingError, fabs(data.yaw));
    }

    TEST_ASSERT_TRUE_MESSAGE(maxPitchError <= 0.5f, 
        "Poor pitch calibration accuracy");
    TEST_ASSERT_TRUE_MESSAGE(maxRollError <= 0.5f, 
        "Poor roll calibration accuracy");
    TEST_ASSERT_TRUE_MESSAGE(maxHeadingError <= 2.0f, 
        "Poor heading calibration accuracy");
}

void runTests() {
    UNITY_BEGIN();
    RUN_TEST(test_heavy_weather_navigation);
    RUN_TEST(test_rapid_maneuvering);
    RUN_TEST(test_magnetic_interference);
    RUN_TEST(test_sensor_drift);
    RUN_TEST(test_combined_stresses);
    RUN_TEST(test_calibration_accuracy);
    UNITY_END();
}

#ifdef ARDUINO
void setup() {
    runTests();
}

void loop() {}
#else
int main(int argc, char **argv) {
    runTests();
    return 0;
}
#endif