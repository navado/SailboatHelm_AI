#include <unity.h>
#include "IMUFilterAndCalibration.h"
#include <cmath>

class SimulatedIMUProvider : public IIMUProvider {
private:
    float _time;
    const float _pitchAmplitude = 5.0f;  // degrees
    const float _rollAmplitude = 10.0f;   // degrees
    const float _pitchPeriod = 4.0f;    // seconds
    const float _rollPeriod = 6.0f;     // seconds
    const float _trueHeading = 45.0f;   // degrees

public:
    SimulatedIMUProvider() : _time(0.0f) {}

    bool getIMUData(IMUData& data) override {
        // Simulate periodic motion
        float pitch = _pitchAmplitude * sin(2 * M_PI * _time / _pitchPeriod);
        float roll = _rollAmplitude * sin(2 * M_PI * _time / _rollPeriod);
        
        // Convert heading to magnetic readings with compensation
        float heading_rad = _trueHeading * M_PI / 180.0f;
        float pitch_rad = pitch * M_PI / 180.0f;
        float roll_rad = roll * M_PI / 180.0f;

        // Simulate magnetometer readings considering tilt
        data.mx = cos(heading_rad) * cos(pitch_rad) + 
                 sin(heading_rad) * sin(roll_rad) * sin(pitch_rad);
        data.my = sin(heading_rad) * cos(roll_rad);
        data.mz = -cos(heading_rad) * sin(pitch_rad) + 
                  sin(heading_rad) * sin(roll_rad) * cos(pitch_rad);

        // Accelerometer readings
        data.ax = sin(pitch_rad);
        data.ay = -cos(pitch_rad) * sin(roll_rad);
        data.az = -cos(pitch_rad) * cos(roll_rad);

        // Angular rates (derivatives of motion)
        data.gx = _pitchAmplitude * (2 * M_PI / _pitchPeriod) * 
                 cos(2 * M_PI * _time / _pitchPeriod);
        data.gy = _rollAmplitude * (2 * M_PI / _rollPeriod) * 
                 cos(2 * M_PI * _time / _rollPeriod);
        data.gz = 0.0f;  // No yaw rate

        _time += 0.01f;  // 10ms increment
        return true;
    }
};

class SimpleTimeProvider : public ITimeProvider {
    uint64_t _time = 0;
public:
    uint64_t getMillis() const override { return _time; }
    void increment(uint64_t delta) { _time += delta; }
};

void test_heading_stability_under_motion() {
    SimulatedIMUProvider imu;
    SimpleTimeProvider timeProvider;
    IMUFilterAndCalibration filter(imu, timeProvider);

    const int numSamples = 1000;  // 10 seconds at 100Hz
    float headings[numSamples];
    
    // Collect heading data over time
    for(int i = 0; i < numSamples; i++) {
        timeProvider.increment(10);  // 10ms steps
        filter.update();
        FilteredIMUData data = filter.getFilteredData();
        headings[i] = data.yaw;
    }

    // Calculate statistics
    float sum = 0.0f;
    float sumSq = 0.0f;
    for(int i = 0; i < numSamples; i++) {
        sum += headings[i];
        sumSq += headings[i] * headings[i];
    }
    
    float mean = sum / numSamples;
    float variance = (sumSq - (sum * sum / numSamples)) / (numSamples - 1);
    float stdDev = sqrt(variance);

    // Verify heading stability
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 45.0f, mean);  // Mean should be close to true heading
    TEST_ASSERT_TRUE(stdDev < 1.0f);  // Standard deviation should be small
}

void test_pitch_roll_tracking() {
    SimulatedIMUProvider imu;
    SimpleTimeProvider timeProvider;
    IMUFilterAndCalibration filter(imu, timeProvider);

    // Test at peaks of motion
    const float dt = 0.01f;  // 10ms
    const int steps = 400;   // 4 seconds
    
    for(int i = 0; i < steps; i++) {
        timeProvider.increment(10);
        filter.update();
    }
    
    FilteredIMUData data = filter.getFilteredData();
    
    // Verify pitch and roll are being tracked
    // Note: Actual values depend on filter implementation
    TEST_ASSERT_TRUE(fabs(data.pitch) <= 5.5f);  // Max pitch ±5°
    TEST_ASSERT_TRUE(fabs(data.roll) <= 10.5f);  // Max roll ±10°
}

void runTests() {
    UNITY_BEGIN();
    RUN_TEST(test_heading_stability_under_motion);
    RUN_TEST(test_pitch_roll_tracking);
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