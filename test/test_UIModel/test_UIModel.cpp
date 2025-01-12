#include <Arduino.h>
#include <unity.h>
#include "UIModel.h"

static UIModel model;

void setUp() {
    // runs before each test
}

void tearDown() {
    // runs after each test
}

void test_initial_values() {
    TEST_ASSERT_EQUAL(UIAutoMode::STANDBY, model.getState().autoMode);
    TEST_ASSERT_EQUAL_STRING("OFF", model.getState().currentSteeringMode.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 90.0f, model.getState().headingSetpoint);
}

void test_increment_small() {
    float oldVal = model.getState().headingSetpoint;
    model.incrementSetpointSmall();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, oldVal+1.0f, model.getState().headingSetpoint);
}

void test_increment_large() {
    float oldVal = model.getState().headingSetpoint;
    model.incrementSetpointLarge();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, oldVal+10.0f, model.getState().headingSetpoint);
}

#ifdef ARDUINO
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_initial_values);
    RUN_TEST(test_increment_small);
    RUN_TEST(test_increment_large);
    UNITY_END();
}

void loop() {
    // not used
}
#endif
