#include <unity.h>
#include "UIModel.h"

static UIModel model;

void setUp() {
    // runs before each test
}
void tearDown() {
    // runs after each test
}

void test_initial_state() {
    auto s = model.getState();
    TEST_ASSERT_EQUAL(UIAutoMode::STANDBY, s.autoMode);
    TEST_ASSERT_EQUAL_STRING("OFF", s.currentSteeringMode.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 90.0f, s.headingSetpoint);
}

void test_increment_small() {
    float oldVal = model.getState().headingSetpoint;
    model.incrementSetpointSmall();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, oldVal+1.0f, model.getState().headingSetpoint);
}

#ifdef ARDUINO
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_initial_state);
    RUN_TEST(test_increment_small);
    UNITY_END();
}
void loop() {}
#endif
