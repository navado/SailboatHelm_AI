#include <unity.h>
#include "IMUFilterAndCalibration.h"

// Mock classes
class MockIMUProvider : public IIMUProvider {
public:
    bool newData = false;
    IMUData data;

    bool getIMUData(IMUData& outData) override {
        if(!newData) return false;
        outData = data;
        newData = false;
        return true;
    }
};

class MockTimeProvider : public ITimeProvider {
public:
    std::uint64_t currentMs = 0;
    std::uint64_t getMillis() const override {
        return currentMs;
    }
};

static MockIMUProvider mockImu;
static MockTimeProvider mockTime;
static IMUFilterAndCalibration filterCal(mockImu, mockTime);

void setUp() {}
void tearDown() {}

void test_no_new_data_no_update() {
    mockImu.newData=false;
    mockTime.currentMs=1000;
    filterCal.update();
    FilteredIMUData fd = filterCal.getFilteredData();
    // no changes
    TEST_ASSERT_EQUAL_FLOAT(0.f, fd.roll);
}

void test_integration_simple() {
    // Provide some gyro data
    mockImu.newData=true;
    mockImu.data = new IMUData();
    mockImu.data->gx = 0.1f;
    mockTime.currentMs=2000;
    filterCal.update();
    // do it again with small dt
    mockImu.newData=true;
    mockTime.currentMs=2100;
    filterCal.update();
    FilteredIMUData fd = filterCal.getFilteredData();
    TEST_ASSERT_NOT_EQUAL(0.f, fd.roll);
}

#ifdef ARDUINO
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_no_new_data_no_update);
    RUN_TEST(test_integration_simple);
    UNITY_END();
}
void loop() {}
#else
int main(){
    UNITY_BEGIN();
    RUN_TEST(test_no_new_data_no_update);
    RUN_TEST(test_integration_simple);
    return UNITY_END();
}
#endif
