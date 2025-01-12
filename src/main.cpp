#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>

// Your classes:
#include "IMUFilterAndCalibration.h"
#include "AutoSteeringController.h"
#include "RudderPositionController.h"

/***************************************************************
 * Hardware pins, e.g. for ESP32
 ***************************************************************/
static const int PIN_IMU_SDA  = 21;
static const int PIN_IMU_SCL  = 22;
static const int PIN_IMU_INT  = 27;  // data-ready pin
static const int PIN_MOTOR_A  = 25;
static const int PIN_MOTOR_B  = 26;
static const int PIN_RUDDER_FEEDBACK = 34;

/***************************************************************
 * Global objects
 ***************************************************************/
IMUFilterAndCalibration imuFilter;
AutoSteeringController  autoSteer;
RudderPositionController rudderCtrl(PIN_MOTOR_A, PIN_MOTOR_B, PIN_RUDDER_FEEDBACK);

// IMU interrupt data-ready flag
volatile bool g_imuDataReady=false;
void IRAM_ATTR onIMUInterrupt(){
    g_imuDataReady=true;
}

/***************************************************************
 * Wi-Fi credentials (example)
 ***************************************************************/
const char* WIFI_SSID="MyBoatWiFi";
const char* WIFI_PASS="Sail1234";

/***************************************************************
 * Setup
 ***************************************************************/
void setup(){
    Serial.begin(115200);
    delay(1000);

    // 1) Filesystem
    LittleFS.begin(true);

    // 2) Start IMU
    if(!imuFilter.begin(PIN_IMU_SDA, PIN_IMU_SCL, PIN_IMU_INT)){
        Serial.println("IMU init failed. Halting.");
        while(true) { delay(100); }
    }
    pinMode(PIN_IMU_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_IMU_INT), onIMUInterrupt, RISING);

    // 3) Start rudder controller
    rudderCtrl.begin();

    // 4) Optionally connect Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting Wi-Fi...");
    while(WiFi.status()!=WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected. IP=%s\n", WiFi.localIP().toString().c_str());

    // 5) By default, let's do track heading 90 deg
    autoSteer.setMode(AutoSteeringMode::TRACK_HEADING, 90.0f);

    Serial.println("Setup complete.\n");
}

/***************************************************************
 * Helper function to do a "soft transition" into RUDDER_LOCK.
 * We read the current rudder angle, then ramp from that angle
 * to the new lock angle in small increments.
 ***************************************************************/
void softTransitionToRudderLock(float newLockAngle){
    // clamp to safe range
    if(newLockAngle>25.0f) newLockAngle=25.0f;
    if(newLockAngle<-25.0f) newLockAngle=-25.0f;

    // read the current rudder angle
    float currentAngle = rudderCtrl.getCurrentAngle();
    // We'll ramp from currentAngle to newLockAngle in steps
    int steps = 20; // e.g. 20 steps
    float stepSize = (newLockAngle - currentAngle)/steps;

    for(int i=0; i<steps; i++){
        float intermediate = currentAngle + stepSize*(i+1);
        // set autopilot to RUDDER_LOCK, but with the intermediate angle
        autoSteer.setMode(AutoSteeringMode::RUDDER_LOCK, intermediate);
        // call update logic a few times
        for(int j=0;j<5;j++){
            // minimal loop
            autoSteer.update();
            rudderCtrl.setTargetAngle(autoSteer.getDesiredRudderAngle());
            rudderCtrl.update();
            delay(20);
        }
    }
    // finally set exact lock angle
    autoSteer.setMode(AutoSteeringMode::RUDDER_LOCK, newLockAngle);
    Serial.printf("[Main] Soft transition done. Rudder locked at %.2f\n", newLockAngle);
}

/***************************************************************
 * Main Loop
 ***************************************************************/
void loop(){
    // 1) If IMU data is ready, update the IMU filter
    if(g_imuDataReady){
        noInterrupts();
        g_imuDataReady=false;
        interrupts();
        imuFilter.update();
    }

    // 2) Periodically feed IMU data to autopilot environment
    static unsigned long envTimer=0;
    if(millis()-envTimer >= 50){
        envTimer=millis();
        FilteredIMUData fd=imuFilter.getFilteredData();

        // Fill environment data
        EnvironmentData env;
        env.heading=fd.yaw;        // simplistic usage
        env.course=0.0f;           // or from GPS
        env.windDirection=0.0f;    // or real wind sensor
        autoSteer.updateEnvironmentData(env);
    }

    // 3) Update autopilot logic
    autoSteer.update();

    // 4) Retrieve desired rudder angle and feed to rudder controller
    float desiredAngle = autoSteer.getDesiredRudderAngle();
    rudderCtrl.setTargetAngle(desiredAngle);
    rudderCtrl.update();

    // EXAMPLE: If user wants to lock rudder at +10 deg after some time
    //  (In real code, you might do this on a button press or command)
    static bool lockApplied=false;
    if(!lockApplied && millis()>30000){
        lockApplied=true;
        Serial.println("[Main] Engaging soft rudder lock => +10 deg");
        softTransitionToRudderLock(+10.0f);  // soft transition
    }

    delay(20); 
}
