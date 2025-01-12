// test/test_AutoSteeringController/test_AutoSteeringController.cpp

#include <Arduino.h>
#include <unity.h>
#include "AutoSteeringController.h"

// If we want to control time, we can define a global variable or do not define it at all:
static unsigned long fakeTime = 0;

// We do NOT redefine 'millis()' if we want the real function to run. 
// Or we can do:
#if 0
extern "C" unsigned long millis() {
    return fakeTime;
}
void incrementFakeTime(unsigned long ms) {
    fakeTime += ms;
}
#endif

static AutoSteeringController autoSteer;

void setUp() {
    autoSteer.setMode(AutoSteeringMode::OFF);
}

void tearDown() {
    // ...
}

void test_off_mode_zero_rudder() {
    autoSteer.update();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, autoSteer.getDesiredRudderAngle());
}

void test_rudder_lock_clamps_angle() {
    autoSteer.setMode(AutoSteeringMode::RUDDER_LOCK, 30.0f);
    autoSteer.update();
    // Should clamp to 25 deg
    TEST_ASSERT_EQUAL_FLOAT(25.0f, autoSteer.getDesiredRudderAngle());
}

void test_rudder_lock_valid_angle() {
    autoSteer.setMode(AutoSteeringMode::RUDDER_LOCK, 10.0f);
    autoSteer.update();
    TEST_ASSERT_EQUAL_FLOAT(10.0f, autoSteer.getDesiredRudderAngle());
}

#ifdef ARDUINO
#if 1
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_off_mode_zero_rudder);
    RUN_TEST(test_rudder_lock_clamps_angle);
    RUN_TEST(test_rudder_lock_valid_angle);
    UNITY_END();
}
void loop(){}
#endif
#endif
