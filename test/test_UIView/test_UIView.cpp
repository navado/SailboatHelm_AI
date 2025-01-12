#include <Arduino.h>
#include <unity.h>
#include "UIView.h"
#include "UIModel.h"

// We won't do a real I2C test here, but we can check that 'begin()' works.
static UIView view;
static UIModel model;

void setUp() {}
void tearDown() {}

void test_ui_view_begin() {
    bool ok = view.begin();  // might init the display
    TEST_ASSERT_TRUE(ok);
}

void test_ui_view_render() {
    // We'll set the model to some values
    model.setAutoMode(UIAutoMode::AUTO);
    model.setSteeringMode("TRACK_HEADING");
    model.setHeadingSetpoint(123.4f);

    // Just call render, ensure no crash
    view.render(model);

    // In a more advanced scenario, we might mock the u8g2 calls
    // to verify correct strings, etc.
    TEST_PASS_MESSAGE("UIView render test passed (no crash).");
}

#ifdef ARDUINO
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_ui_view_begin);
    RUN_TEST(test_ui_view_render);
    UNITY_END();
}

void loop() {}
#endif
