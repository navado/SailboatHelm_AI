#include <unity.h>
#include "IMUFilterAndCalibration.h"
#include <cmath>

class AdvancedIMUProvider : public IIMUProvider {
private:
    float _time;
    float _lastHeading;
    

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
    };

    MotionScenario _scenario;

    AdvancedIMUProvider(const MotionScenario& scenario) 
        : _time(0.0f), _scenario(scenario), _lastHeading(scenario.initialHeading) {}

    bool getIMUData(IMUData& data) override {
        // Basic motion
        float pitch = _scenario.pitchAmplitude * sin(2 * M_PI * _time / _scenario.pitchPeriod);
        float roll = _scenario.rollAmplitude * sin(2 * M_PI * _time / _scenario.rollPeriod);
        
        // Add wave impact
        float waveOffset = _scenario.waveHeight * 
            sin(2 * M_PI * _time / _scenario.wavePeriod);
        pitch += waveOffset * cos(2 * M_PI * _time / 1.5f); // Fast oscillation
        roll += waveOffset * sin(2 * M_PI * _time / 2.0f);  // Different phase

        // Add sudden jerk if configured
        if (_scenario.hasJerk && fabs(fmod(_time, 5.0f) - 2.5f) < 0.05f) {
            pitch += 3.0f * sin(2 * M_PI * _time * 10);
            roll += 2.0f * sin(2 * M_PI * _time * 12);
        }

        // Update heading with yaw rate
        _lastHeading += _scenario.yawRate * 0.01f; // 10ms steps
        if (_lastHeading >= 360.0f) _lastHeading -= 360.0f;
        
        float heading_rad = _lastHeading * M_PI / 180.0f;
        float pitch_rad = pitch * M_PI / 180.0f;
        float roll_rad = roll * M_PI / 180.0f;

        // Magnetometer with tilt compensation
        data.mx = cos(heading_rad) * cos(pitch_rad) + 
                 sin(heading_rad) * sin(roll_rad) * sin(pitch_rad);
        data.my = sin(heading_rad) * cos(roll_rad);
        data.mz = -cos(heading_rad) * sin(pitch_rad) + 
                  sin(heading_rad) * sin(roll_rad) * cos(pitch_rad);

        // Accelerometer with wave motion
        float baseAx = sin(pitch_rad);
        float baseAy = -cos(pitch_rad) * sin(roll_rad);
        float baseAz = -cos(pitch_rad) * cos(roll_rad);
        
        data.ax = baseAx + waveOffset * 0.1f * sin(2 * M_PI * _time / 0.5f);
        data.ay = baseAy + waveOffset * 0.1f * cos(2 * M_PI * _time / 0.6f);
        data.az = baseAz + waveOffset * 0.15f * sin(2 * M_PI * _time / 0.4f);

        // Gyro rates with wave impact
        data.gx = _scenario.pitchAmplitude * (2 * M_PI / _scenario.pitchPeriod) * 
                 cos(2 * M_PI * _time / _scenario.pitchPeriod);
        data.gy = _scenario.rollAmplitude * (2 * M_PI / _scenario.rollPeriod) * 
                 cos(2 * M_PI * _time / _scenario.rollPeriod);
        data.gz = _scenario.yawRate * M_PI / 180.0f; // deg/s to rad/s

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
 * Scenario 1: Heavy Weather Navigation
 * - Large wave height (3m) with 8s period
 * - Significant pitch (±15°) and roll (±25°) motion
 * - Slow turn rate (2°/s)
 * - Random jerks from wave impacts
 * Tests filter's ability to maintain heading accuracy in challenging conditions
 */
void test_heavy_weather_navigation() {
    AdvancedIMUProvider::MotionScenario heavyWeather = {
        .pitchAmplitude = 15.0f,
        .rollAmplitude = 25.0f,
        .yawRate = 2.0f,
        .pitchPeriod = 4.0f,
        .rollPeriod = 6.0f,
        .initialHeading = 45.0f,
        .waveHeight = 3.0f,
        .wavePeriod = 8.0f,
        .hasJerk = true
    };

    AdvancedIMUProvider imu(heavyWeather);
    PreciseTimeProvider timeProvider;
    IMUFilterAndCalibration filter(imu, timeProvider);

    const int numSamples = 2000; // 20 seconds
    float maxHeadingError = 0.0f;
    float lastHeading = heavyWeather.initialHeading;
    
    for(int i = 0; i < numSamples; i++) {
        timeProvider.increment(10);
        filter.update();
        FilteredIMUData data = filter.getFilteredData();
        
        // Calculate expected heading
        float expectedHeading = heavyWeather.initialHeading + 
                              heavyWeather.yawRate * (i * 0.01f);
        while(expectedHeading >= 360.0f) expectedHeading -= 360.0f;
        
        // Calculate heading error considering wraparound
        float error = fabs(data.yaw - expectedHeading);
        if(error > 180.0f) error = 360.0f - error;
        
        maxHeadingError = fmax(maxHeadingError, error);
        
        // Verify maximum heading deviation
        TEST_ASSERT_TRUE_MESSAGE(error <= 5.0f, 
            "Heading deviation exceeded 5 degrees in heavy weather");

        // Verify no sudden heading jumps
        float headingDelta = fabs(data.yaw - lastHeading);
        if(headingDelta > 180.0f) headingDelta = 360.0f - headingDelta;
        
        TEST_ASSERT_TRUE_MESSAGE(headingDelta <= 2.0f, 
            "Sudden heading change detected");
        
        lastHeading = data.yaw;
    }
}

void runTests() {
    UNITY_BEGIN();
    RUN_TEST(test_heavy_weather_navigation);
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