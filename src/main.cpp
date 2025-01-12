#include <Arduino.h>
#include "IIMUProvider.h"
#include "MyIMUProvider.h"
#include "AutoSteeringController.h"
#include "IMUFilterAndCalibration.h"
#include "UIModel.h"
#include "UIView.h"
#include "UIController.h"
#include "IInputDevice.h"
#include "ITimeProvider.h"

// Pins for UI buttons, etc.
static const int PIN_BTN_AUTO = 2;
// ... other pins ...

// MyTimeProvider
class MyTimeProvider : public ITimeProvider {
public:
    std::uint64_t getMillis() const override {
        return (std::uint64_t)millis();
    }
};

// MyInputDevice
class MyInputDevice : public IInputDevice {
public:
    bool isPressed(ButtonId btn) const override {
        // switch/case, digitalRead
        // ...
        return false;
    }
};

// Global Instances
static AutoSteeringController autoSteer;
static MyTimeProvider timeProv;
static MyIMUProvider  imuProv;  // extracted to separate file
static IMUFilterAndCalibration imuFilter(imuProv, timeProv);

static UIModel uiModel;
static UIView  uiView;
static MyInputDevice inputDev;
static UIController uiController(uiModel, autoSteer, inputDev);

// Timer approach for 100Hz IMU
hw_timer_t* g_imuTimer=nullptr;

void IRAM_ATTR onIMUTimer(){
    // call filter update at 100Hz
    imuFilter.update();
}

void setup() {
    Serial.begin(115200);

    // pinMode for UI
    pinMode(PIN_BTN_AUTO, INPUT_PULLUP);

    // Start IMU
    imuProv.begin(); // references Wire, attachInterrupt, etc.

    // Start UI
    uiView.begin();

    // Create hardware timer for 100Hz
    g_imuTimer = timerBegin(0, 80, true);
    timerAttachInterrupt(g_imuTimer, &onIMUTimer, true);
    timerAlarmWrite(g_imuTimer, 10000, true);
    timerAlarmEnable(g_imuTimer);

    Serial.println("Setup done.");
}

void loop() {
    // Possibly autopilot update
    static unsigned long lastAuto=0;
    unsigned long now=millis();
    if((now-lastAuto)>100) {
        lastAuto=now;
        autoSteer.update(0.1f);
    }

    // UI update
    uiController.update();
    uiView.render(uiModel);

    delay(50);
}
