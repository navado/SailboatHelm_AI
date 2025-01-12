// test/test_IMUFilterAndCalibration/test_IMUFilterAndCalibration.cpp

#include <Arduino.h>
#include <unity.h>

// Include the class under test
#include "IMUFilterAndCalibration.h"

// If you have a custom or minimal MPU9250 library, include that instead.
// If you need to mock I2C or 'Wire', do it here or in a separate mock file.

static IMUFilterAndCalibration imuFilter;

void setUp(void) {
    // Runs before each test
}

void tearDown(void) {
    // Runs after each test
}

void test_imu_begin_ok() {
    // If 'begin()' is expected to succeed under test conditions
    bool result = imuFilter.begin(21, 22, 27);
    TEST_ASSERT_TRUE_MESSAGE(result, "IMUFilterAndCalibration::begin() should return true if init is successful");
}

void test_imu_update_no_crash() {
    imuFilter.update();
    // We can check default orientation after an update
    auto data = imuFilter.getFilteredData();
    // Depending on your filter logic, might be near 0
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, data.roll);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, data.pitch);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, data.yaw);
}

void test_imu_calibration_flow() {
    imuFilter.startCalibration();
    imuFilter.doCalibrationStep();
    // In real tests, you'd mock out the file writes, ensuring it calls saveCalibration().
    TEST_PASS_MESSAGE("Calibration step test passed (no crash).");
}

// Register tests with Unity
// We do NOT define setup() / loop() here. 
// PlatformIO's test runner calls these test functions automatically.
#ifdef ARDUINO
// If you prefer to control the order manually, you can wrap them:
#if 0
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_imu_begin_ok);
    RUN_TEST(test_imu_update_no_crash);
    RUN_TEST(test_imu_calibration_flow);
    UNITY_END();
}

void loop() {
    // Not used
}
#endif
#endif
