#include <unity.h>
#include "AutoSteeringController.h"
#include <cmath>

// Test Variables
static AutoSteeringController* controller = nullptr;
const float maxRudderAngle = 30.0f;
const float dt = 0.1f;  // 10Hz update rate

void setUp(void) {
    controller = new AutoSteeringController();
}

void tearDown(void) {
    delete controller;
    controller = nullptr;
}

/*
 * Test heading tracking with different heading errors:
 * - Small error (<5°)
 * - Medium error (45°)
 * - Large error (180°)
 */
void test_heading_tracking_response(void) {
    // Test small error
    controller->setMode(AutoSteeringMode::TRACK_HEADING, 5.0f);
    controller->update(dt);
    float smallError = abs(controller->getRudderAngle());
    
    // Test medium error
    controller->setMode(AutoSteeringMode::TRACK_HEADING, 45.0f);
    controller->update(dt);
    float mediumError = abs(controller->getRudderAngle());
    
    // Test large error
    controller->setMode(AutoSteeringMode::TRACK_HEADING, 180.0f);
    controller->update(dt);
    float largeError = abs(controller->getRudderAngle());
    
    TEST_ASSERT_TRUE(smallError < mediumError);
    TEST_ASSERT_TRUE(mediumError < largeError);
    TEST_ASSERT_TRUE(largeError <= maxRudderAngle);
}

/*
 * Tests mode transitions:
 * - OFF -> TRACK_HEADING -> TRACK_COURSE -> TRACK_WIND_ANGLE -> OFF
 */
void test_mode_transitions(void) {
    float lastRudder = 0.0f;
    
    controller->setMode(AutoSteeringMode::OFF, 0.0f);
    controller->update(dt);
    lastRudder = controller->getRudderAngle();
    
    const AutoSteeringMode modes[] = {
        AutoSteeringMode::TRACK_HEADING,
        AutoSteeringMode::TRACK_COURSE,
        AutoSteeringMode::TRACK_WIND_ANGLE,
        AutoSteeringMode::OFF
    };
    const float setpoints[] = {45.0f, 90.0f, 30.0f, 0.0f};
    
    for(int i = 0; i < 4; i++) {
        controller->setMode(modes[i], setpoints[i]);
        controller->update(dt);
        float newRudder = controller->getRudderAngle();
        float rudderJump = abs(newRudder - lastRudder);
        
        TEST_ASSERT_TRUE_MESSAGE(rudderJump <= 5.0f, 
            "Excessive rudder movement during mode transition");
        lastRudder = newRudder;
    }
}

void test_rudder_smoothness(void) {
    const int numSteps = 100;
    float maxRateChange = 0.0f;
    float lastRudder = controller->getRudderAngle();

    controller->setMode(AutoSteeringMode::TRACK_HEADING, 90.0f);

    for(int i = 0; i < numSteps; i++) {
        controller->update(dt);
        float newRudder = controller->getRudderAngle();
        float rateChange = abs(newRudder - lastRudder) / dt;
        
        // Check for excessive rate changes
        TEST_ASSERT_TRUE(rateChange <= 5.0f);

        maxRateChange = fmax(maxRateChange, rateChange);
        lastRudder = newRudder;
    }

    TEST_ASSERT_TRUE_MESSAGE(maxRateChange <= 20.0f, 
        "Rudder rate exceeds safe limits");
}


/*
 * Tests PID controller behavior:
 * - Proportional response
 * - Integral windup prevention
 * - Derivative kick mitigation
 */
void test_pid_behavior(void) {
    // Test P term
    controller->setMode(AutoSteeringMode::TRACK_HEADING, 10.0f);
    controller->update(dt);
    float p_response = controller->getRudderAngle();
    
    // Test I accumulation
    float i_sum = 0.0f;
    for(int i = 0; i < 50; i++) {
        controller->update(dt);
        i_sum += controller->getRudderAngle();
    }
    
    // Test D response to sudden change
    controller->setMode(AutoSteeringMode::TRACK_HEADING, 20.0f);
    controller->update(0.01f);  // Short dt to amplify D term
    float d_response = abs(controller->getRudderAngle());
    
    TEST_ASSERT_TRUE(p_response != 0.0f);
    TEST_ASSERT_TRUE(i_sum < 50 * maxRudderAngle);  // Check for windup limit
    TEST_ASSERT_TRUE(d_response <= maxRudderAngle);
}

/*
 * Tests rudder response smoothness:
 * - Continuous motion
 * - Rate limiting
 */
void test_rudder_smoothness(void) {
    const int numSteps = 100;
    float maxRateChange = 0.0f;
    float lastRudder = 0.0f;
    
    controller->setMode(AutoSteeringMode::TRACK_HEADING, 90.0f);
    
    for(int i = 0; i < numSteps; i++) {
        controller->update(dt);
        float newRudder = controller->getRudderAngle();
        float rateChange = abs(newRudder - lastRudder) / dt;
        maxRateChange = fmax(maxRateChange, rateChange);
        lastRudder = newRudder;
    }
    
    TEST_ASSERT_TRUE_MESSAGE(maxRateChange <= 20.0f, 
        "Rudder rate exceeds safe limits");
}

void test_wind_angle_compensation(void) {
    // Compensate for pitch and roll. Calculate effect based on mast height and vessel length.
    // The wind tracker should be able to compensate for the wind angle based on the vessel's pitch and roll.
    // Giving commands to the rudder to compensate for the wind angle changes due to pitch and roll
    // Test the reaction based on expected vector changes
    // As test procedure, mix influence of pitch and roll on the wind angle and check if the rudder reacts accordingly

    const float mastHeight = 10.0f;  // Example mast height
    const float vesselLength = 20.0f;  // Example vessel length
    const float windAngles[] = {30.0f, 45.0f, 60.0f};  // Different wind angles
    const float pitchValues[] = {5.0f, 10.0f};  // Different pitch values
    const float rollValues[] = {3.0f, 6.0f};  // Different roll values

    for (float windAngle : windAngles) {
        for (float pitch : pitchValues) {
            for (float roll : rollValues) {
                // Simulate wind angle compensation
                controller->setMode(AutoSteeringMode::TRACK_WIND_ANGLE, windAngle);
                controller->update(dt);

                float initialRudder = controller->getRudderAngle();

                // Apply pitch and roll influence using circular coordinates
                float windAngleChangeDueToPitch = pitch * (mastHeight / vesselLength);
                float windAngleChangeDueToRoll = roll * (mastHeight / vesselLength);

                float totalWindAngleChange = sqrt(pow(windAngleChangeDueToPitch, 2) + pow(windAngleChangeDueToRoll, 2));

                controller->setMode(AutoSteeringMode::TRACK_WIND_ANGLE, windAngle + totalWindAngleChange);
                controller->update(dt);

                float compensatedRudder = controller->getRudderAngle();

                TEST_ASSERT_TRUE_MESSAGE(abs(compensatedRudder - initialRudder) <= maxRudderAngle, 
                    "Rudder compensation for wind angle due to pitch and roll is incorrect");
            }
        }
    }
}

/*
 * Tests wind angle tracking:
 * - Different wind angles
 * - Wind shifts
 */
void test_wind_angle_tracking(void) {
    const float windAngles[] = {30.0f, 45.0f, 60.0f, 90.0f, 120.0f, 150.0f};
    const int numAngles = sizeof(windAngles) / sizeof(windAngles[0]);
    
    for(int i = 0; i < numAngles; i++) {
        controller->setMode(AutoSteeringMode::TRACK_WIND_ANGLE, windAngles[i]);
        controller->update(dt);
        float rudder = abs(controller->getRudderAngle());
        TEST_ASSERT_TRUE(rudder <= maxRudderAngle);
    }
    
    // Test wind shift response
    controller->setMode(AutoSteeringMode::TRACK_WIND_ANGLE, 45.0f);
    float lastRudder = controller->getRudderAngle();
    
    for(int i = 0; i < 50; i++) {
        controller->update(dt);
        float newRudder = controller->getRudderAngle();
        float rudderJump = abs(newRudder - lastRudder);
        TEST_ASSERT_TRUE(rudderJump <= 5.0f);
        lastRudder = newRudder;
    }
}

/*
 * Tests course tracking stability
 */
void test_course_tracking(void) {
    controller->setMode(AutoSteeringMode::TRACK_COURSE, 45.0f);
    float maxRudder = 0.0f;
    float minRudder = 0.0f;
    
    for(int i = 0; i < 100; i++) {
        controller->update(dt);
        float rudder = controller->getRudderAngle();
        maxRudder = fmax(maxRudder, rudder);
        minRudder = fmin(minRudder, rudder);
    }
    
    float rudderRange = maxRudder - minRudder;
    TEST_ASSERT_TRUE_MESSAGE(rudderRange <= 10.0f, 
        "Excessive rudder movement in course tracking");
}

int runTests() {
    UNITY_BEGIN();
    RUN_TEST(test_heading_tracking_response);
    RUN_TEST(test_mode_transitions);
    RUN_TEST(test_pid_behavior);
    RUN_TEST(test_rudder_smoothness);
    RUN_TEST(test_wind_angle_tracking);
    RUN_TEST(test_course_tracking);
    return UNITY_END();
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