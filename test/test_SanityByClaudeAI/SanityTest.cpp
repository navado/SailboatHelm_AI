#include <unity.h>
#include "AutoSteeringController.h"
#include "UIModel.h"
#include "IMUFilterAndCalibration.h"
#include "RudderPositionController.h"

// Mock Implementation
class MockIMUProvider : public IIMUProvider {
private:
    IMUData _mockData;
    bool _shouldReturnData;
public:
    MockIMUProvider() : _shouldReturnData(true) {
        _mockData.ax = 0.1f;
        _mockData.ay = 0.2f;
        _mockData.az = 9.81f;
        _mockData.gx = 0.0f;
        _mockData.gy = 0.0f;
        _mockData.gz = 0.0f;
        _mockData.mx = 0.0f;
        _mockData.my = 0.0f;
        _mockData.mz = 0.0f;
    }
    bool getIMUData(IMUData& data) override {
        if (_shouldReturnData) {
            data = _mockData;
            return true;
        }
        return false;
    }
    void setMockData(const IMUData& data) { _mockData = data; }
    void setShouldReturnData(bool val) { _shouldReturnData = val; }
};

class MockTimeProvider : public ITimeProvider {
private:
    uint64_t _currentTime;
public:
    MockTimeProvider() : _currentTime(0) {}
    uint64_t getMillis() const override { return _currentTime; }
    void setTime(uint64_t time) { _currentTime = time; }
};

// Test Variables
AutoSteeringController* autoSteer;
UIModel* uiModel;
MockIMUProvider* mockImu;
MockTimeProvider* mockTime;
IMUFilterAndCalibration* imuFilter;

void setUp(void) {
    autoSteer = new AutoSteeringController();
    uiModel = new UIModel();
    mockImu = new MockIMUProvider();
    mockTime = new MockTimeProvider();
    imuFilter = new IMUFilterAndCalibration(*mockImu, *mockTime);
}

void tearDown(void) {
    delete autoSteer;
    delete uiModel;
    delete mockImu;
    delete mockTime;
    delete imuFilter;
}

void test_autoSteer_initial_state(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, autoSteer->getRudderAngle());
}

void test_autoSteer_heading_mode(void) {
    autoSteer->setMode(AutoSteeringMode::TRACK_HEADING, 180.0f);
    autoSteer->update(0.1f);
    float rudder = autoSteer->getRudderAngle();
    TEST_ASSERT_TRUE(abs(rudder) <= 30.0f);
}

void test_imu_filter_calibration(void) {
    mockTime->setTime(0);
    
    IMUData testData;
    testData.ax = 0.1f;
    testData.ay = 0.2f;
    testData.az = 9.81f;
    testData.gx = 0.0f;
    testData.gy = 0.0f;
    testData.gz = 0.0f;
    testData.mx = 0.0f;
    testData.my = 0.0f;
    testData.mz = 0.0f;
    
    mockImu->setMockData(testData);
    
    imuFilter->startCalibration();
    imuFilter->update();
    
    FilteredIMUData filtered = imuFilter->getFilteredData();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, filtered.pitch);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, filtered.roll);
}

void test_ui_model_default_state(void) {
    const UIState& state = uiModel->getState();
    TEST_ASSERT_EQUAL(UIAutoMode::STANDBY, state.autoMode);
    TEST_ASSERT_EQUAL_STRING("OFF", state.currentSteeringMode.c_str());
    TEST_ASSERT_EQUAL_FLOAT(90.0f, state.headingSetpoint);
}

void test_ui_model_setpoint_changes(void) {
    float initial = uiModel->getState().headingSetpoint;
    uiModel->incrementSetpointSmall();
    TEST_ASSERT_EQUAL_FLOAT(initial + 1.0f, uiModel->getState().headingSetpoint);
}

void runTests() {
    UNITY_BEGIN();
    RUN_TEST(test_autoSteer_initial_state);
    RUN_TEST(test_autoSteer_heading_mode);
    RUN_TEST(test_imu_filter_calibration);
    RUN_TEST(test_ui_model_default_state);
    RUN_TEST(test_ui_model_setpoint_changes);
    UNITY_END();
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    runTests();
}

void loop() {}
#else
int main(int argc, char **argv) {
    runTests();
    return 0;
}
#endif