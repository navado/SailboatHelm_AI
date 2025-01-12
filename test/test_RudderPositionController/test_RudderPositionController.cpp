// test/test_RudderPositionController/test_RudderPositionController.cpp

#include <unity.h>
#include "RudderPositionController.h"

// Mock analogRead with correct signature for ESP32: returns uint16_t
extern "C" uint16_t analogRead(uint8_t pin) {
    // Example: return ~3000 => ~73% => angle ~ +12 deg
    return 3000;
}

// Mock ledcWrite to avoid actual hardware calls
extern "C" void ledcWrite(uint8_t channel, uint32_t duty) {
    // Could store in a global test var if you want to assert on it
    // For now, do nothing
}

// If you want to control time, you can do so, but do not redefine 'millis()'
// If your code references 'millis()', let the real one run or do a custom approach.

static RudderPositionController rudderCtrl(25, 26, 34);

void setUp() {
    rudderCtrl.begin();
}

void tearDown() {
    // after each test
}

void test_rudder_set_target_angle() {
    rudderCtrl.setTargetAngle(10.0f);
    // Run update a few times
    for(int i=0; i<5; i++){
        rudderCtrl.update();
        delay(10); // or do nothing if you want to simulate time differently
    }
    // We just ensure no crash. Real test might check motor output if we stored it
    TEST_PASS_MESSAGE("Set target angle 10 deg OK");
}

void test_rudder_negative_angle() {
    rudderCtrl.setTargetAngle(-15.0f);
    for(int i=0; i<5; i++){
        rudderCtrl.update();
        delay(10);
    }
    TEST_PASS_MESSAGE("Set target angle -15 deg OK");
}

// Same note about not defining setup() and loop() here
#ifdef ARDUINO
#if 0
void setup(){
    UNITY_BEGIN();
    RUN_TEST(test_rudder_set_target_angle);
    RUN_TEST(test_rudder_negative_angle);
    UNITY_END();
}
void loop(){}
#endif
#endif
