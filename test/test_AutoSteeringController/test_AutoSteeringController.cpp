#include <unity.h>
#include "AutoSteeringController.h"

// We'll have a static instance to test
static AutoSteeringController autoSteer;

void setUp() { 
    // runs before each test
}
void tearDown() { 
    // runs after each test
}

void test_off_mode_rudder_zero() {
    autoSteer.setMode(AutoSteeringMode::OFF);
    autoSteer.update(0.1f);
    TEST_ASSERT_EQUAL_FLOAT(0.f, autoSteer.getRudderAngle());
}

void test_track_heading_produces_output() {
    autoSteer.setMode(AutoSteeringMode::TRACK_HEADING, 90.f);
    autoSteer.update(0.1f);
    float rudder = autoSteer.getRudderAngle();
    TEST_ASSERT_NOT_EQUAL(0.f, rudder);
}

#ifdef ARDUINO
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_off_mode_rudder_zero);
    RUN_TEST(test_track_heading_produces_output);
    UNITY_END();
}
void loop() {}
#else
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_off_mode_rudder_zero);
    RUN_TEST(test_track_heading_produces_output);
    return UNITY_END();
}
#endif
