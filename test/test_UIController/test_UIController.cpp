#include <Arduino.h>
#include <unity.h>
#include "UIController.h"
#include "UIModel.h"
#include "AutoSteeringController.h"

// We'll mock digitalRead
extern "C" int digitalRead(uint8_t pin) {
    // Return high by default
    return HIGH;
}

// We can define some special variables to simulate button presses if needed
static bool g_autoBtn = false;

// Then interpret them in digitalRead logic:
int customDigitalRead(uint8_t pin) {
    if(pin==2) return g_autoBtn ? LOW : HIGH;
    return HIGH;
}

// We'll define a local "AutoSteeringController" stub
class FakeAutoSteeringController : public AutoSteeringController {
public:
    bool modeSet = false;
    AutoSteeringMode lastMode = AutoSteeringMode::OFF;
    float lastParam = 0.0f;

    void setMode(AutoSteeringMode mode, float param=0.0f)  {
        modeSet = true;
        lastMode = mode;
        lastParam= param;
    }
};

class MockInputDevice : public IInputDevice {
public:
    bool isPressed(ButtonId id) const override {
        return false;
    }
};

static UIModel model;
static FakeAutoSteeringController fakeAuto;
static MockInputDevice input;
static UIController controller(model, fakeAuto, input);

void setUp() {
    // Overwrite digitalRead with custom function
    // In real usage, might wrap or do #ifdef for mocking
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    controller.begin();
    g_autoBtn=false;
}

void tearDown() {}

void test_auto_toggle() {
    // Initially STANDBY
    TEST_ASSERT_EQUAL(UIAutoMode::STANDBY, model.getState().autoMode);

    // Simulate button press
    g_autoBtn=true;  
    controller.update();
    // Now it should become AUTO
    TEST_ASSERT_EQUAL(UIAutoMode::AUTO, model.getState().autoMode);
    TEST_ASSERT_TRUE(fakeAuto.modeSet);
    TEST_ASSERT_EQUAL(AutoSteeringMode::TRACK_HEADING, fakeAuto.lastMode);

    // Press again
    g_autoBtn=false;
    controller.update(); // button released
    g_autoBtn=true;
    controller.update(); 
    // Should revert to STANDBY
    TEST_ASSERT_EQUAL(UIAutoMode::STANDBY, model.getState().autoMode);
    TEST_ASSERT_EQUAL(AutoSteeringMode::OFF, fakeAuto.lastMode);
}

#ifdef ARDUINO
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_auto_toggle);
    UNITY_END();
}
void loop() {}
#endif
