#include <unity.h>
#include "AutoSteeringController.h"
#include "IMUFilterAndCalibration.h"
#include <cmath>

class IntegratedIMUProvider : public IIMUProvider {
public:
    float _time;
    struct VesselState {
        float heading;        // Current true heading
        float pitch;          // Current pitch
        float roll;          // Current roll
        float turnRate;      // Current rate of turn
        float waveHeight;    // Current wave height affecting vessel
        float wavePeriod;    // Wave period
        float windAngle;     // Relative wind angle
        bool hasJerk;        // Whether sudden motion occurs
    } _state;

    void updateVesselState(float dt) {
        // Update heading based on turn rate
        _state.heading += _state.turnRate * dt;
        if(_state.heading >= 360.0f) _state.heading -= 360.0f;
        if(_state.heading < 0.0f) _state.heading += 360.0f;

        // Wave-induced motion
        float wavePhase = 2 * M_PI * _time / _state.wavePeriod;
        _state.pitch = _state.waveHeight * sin(wavePhase) * 2.0f;  // 2 deg pitch per meter
        _state.roll = _state.waveHeight * cos(wavePhase) * 3.0f;   // 3 deg roll per meter

        // Add jerk if enabled
        if(_state.hasJerk && fmod(_time, 5.0f) < 0.1f) {
            _state.pitch += 5.0f * sin(2 * M_PI * _time * 10);
            _state.roll += 8.0f * sin(2 * M_PI * _time * 12);
        }
    }

    IntegratedIMUProvider() : _time(0.0f) {
        _state.heading = 0.0f;
        _state.pitch = 0.0f;
        _state.roll = 0.0f;
        _state.turnRate = 0.0f;
        _state.waveHeight = 0.0f;
        _state.wavePeriod = 6.0f;
        _state.windAngle = 0.0f;
        _state.hasJerk = false;
    }

    void setTurnRate(float rate) { _state.turnRate = rate; }
    void setWaveConditions(float height, float period) {
        _state.waveHeight = height;
        _state.wavePeriod = period;
    }
    void setWindAngle(float angle) { _state.windAngle = angle; }
    void enableJerk(bool enable) { _state.hasJerk = enable; }
    float getCurrentHeading() const { return _state.heading; }

    bool getIMUData(IMUData& data) override {
        const float dt = 0.01f;  // 100Hz simulation
        updateVesselState(dt);

        // Convert to radians
        float heading_rad = _state.heading * M_PI / 180.0f;
        float pitch_rad = _state.pitch * M_PI / 180.0f;
        float roll_rad = _state.roll * M_PI / 180.0f;

        // Magnetometer simulation
        data.mx = cos(heading_rad) * cos(pitch_rad) + 
                 sin(heading_rad) * sin(roll_rad) * sin(pitch_rad);
        data.my = sin(heading_rad) * cos(roll_rad);
        data.mz = -cos(heading_rad) * sin(pitch_rad) + 
                  sin(heading_rad) * sin(roll_rad) * cos(pitch_rad);

        // Accelerometer - includes gravity and motion
        float ax = sin(pitch_rad);
        float ay = -cos(pitch_rad) * sin(roll_rad);
        float az = -cos(pitch_rad) * cos(roll_rad);
        
        // Add dynamic acceleration
        data.ax = ax + _state.turnRate * 0.01f * sin(heading_rad);
        data.ay = ay + _state.turnRate * 0.01f * cos(heading_rad);
        data.az = az - 1.0f; // Gravity offset

        // Gyroscope
        data.gx = _state.pitch * M_PI / 180.0f;  // Convert to rad/s
        data.gy = _state.roll * M_PI / 180.0f;
        data.gz = _state.turnRate * M_PI / 180.0f;

        _time += dt;
        return true;
    }
};

class TestTimeProvider : public ITimeProvider {
    uint64_t _time = 0;
public:
    uint64_t getMillis() const override { return _time; }
    void increment(uint64_t delta) { _time += delta; }
};

// Test fixtures
static IntegratedIMUProvider* imu = nullptr;
static TestTimeProvider* timeProvider = nullptr;
static IMUFilterAndCalibration* imuFilter = nullptr;
static AutoSteeringController* controller = nullptr;

void setUp(void) {
    imu = new IntegratedIMUProvider();
    timeProvider = new TestTimeProvider();
    imuFilter = new IMUFilterAndCalibration(*imu, *timeProvider);
    controller = new AutoSteeringController();
}

void tearDown(void) {
    delete controller;
    delete imuFilter;
    delete timeProvider;
    delete imu;
}

/*
 * Test 1: Heading Control in Calm Conditions
 * - Set target heading
 * - Verify controller response
 * - Check final heading accuracy
 */
void test_calm_heading_control(void) {
    const float targetHeading = 45.0f;
    const float maxError = 2.0f;
    imu->setTurnRate(0.0f);
    imu->setWaveConditions(0.0f, 6.0f);
    
    controller->setMode(AutoSteeringMode::TRACK_HEADING, targetHeading);

    for(int i = 0; i < 200; i++) {  // 20 seconds
        timeProvider->increment(100); // 100ms steps
        imuFilter->update();
        FilteredIMUData filteredData = imuFilter->getFilteredData();
        
        controller->update(0.1f);
        float rudder = controller->getRudderAngle();
        imu->setTurnRate(-rudder * 0.2f); // Simple rudder effectiveness model
    }

    FilteredIMUData finalData = imuFilter->getFilteredData();
    float finalError = abs(targetHeading - finalData.yaw);
    if(finalError > 180.0f) finalError = 360.0f - finalError;
    
    TEST_ASSERT_TRUE_MESSAGE(finalError <= maxError, 
        "Failed to achieve target heading in calm conditions");
}

/*
 * Test 2: Wave Response
 * - Test heading maintenance in waves
 * - Verify rudder activity limits
 */
void test_wave_response(void) {
    imu->setWaveConditions(2.0f, 6.0f);  // 2m waves, 6s period
    const float targetHeading = 90.0f;
    
    controller->setMode(AutoSteeringMode::TRACK_HEADING, targetHeading);
    float maxRudderRate = 0.0f;
    float lastRudder = 0.0f;

    for(int i = 0; i < 300; i++) {  // 30 seconds
        timeProvider->increment(100);
        imuFilter->update();
        controller->update(0.1f);
        
        float rudder = controller->getRudderAngle();
        float rudderRate = abs(rudder - lastRudder) / 0.1f;
        maxRudderRate = fmax(maxRudderRate, rudderRate);
        lastRudder = rudder;
        
        imu->setTurnRate(-rudder * 0.2f);
    }

    TEST_ASSERT_TRUE_MESSAGE(maxRudderRate <= 15.0f, 
        "Excessive rudder rate in waves");
}

/*
 * Test 3: Wind Angle Tracking
 * - Verify relative wind angle maintenance
 * - Check course stability
 */
void test_wind_angle_tracking(void) {
    const float targetWindAngle = 45.0f;
    imu->setWindAngle(targetWindAngle);
    imu->setWaveConditions(1.0f, 5.0f);

    controller->setMode(AutoSteeringMode::TRACK_WIND_ANGLE, targetWindAngle);

    float maxWindError = 0.0f;
    
    for(int i = 0; i < 400; i++) {  // 40 seconds
        timeProvider->increment(100);
        imuFilter->update();
        controller->update(0.1f);
        
        float rudder = controller->getRudderAngle();
        imu->setTurnRate(-rudder * 0.2f);
        
        float currentHeading = imu->getCurrentHeading();
        float windError = abs(targetWindAngle - 
            (imu->getCurrentHeading() + imu->_state.windAngle));
        if(windError > 180.0f) windError = 360.0f - windError;
        
        maxWindError = fmax(maxWindError, windError);
    }

    TEST_ASSERT_TRUE_MESSAGE(maxWindError <= 10.0f, 
        "Failed to maintain target wind angle");
}

/*
 * Test 4: Disturbance Recovery
 * - Simulate sudden disturbance
 * - Verify recovery response
 */
void test_disturbance_recovery(void) {
    const float targetHeading = 180.0f;
    controller->setMode(AutoSteeringMode::TRACK_HEADING, targetHeading);
    
    // Normal operation
    for(int i = 0; i < 100; i++) {
        timeProvider->increment(100);
        imuFilter->update();
        controller->update(0.1f);
        float rudder = controller->getRudderAngle();
        imu->setTurnRate(-rudder * 0.2f);
    }
    
    // Apply disturbance
    imu->enableJerk(true);
    imu->setTurnRate(20.0f);  // Sudden turn
    
    float maxHeadingError = 0.0f;
    float recoveryTime = 0.0f;
    bool recovered = false;
    
    for(int i = 0; i < 200; i++) {
        timeProvider->increment(100);
        imuFilter->update();
        controller->update(0.1f);
        
        float rudder = controller->getRudderAngle();
        imu->setTurnRate(-rudder * 0.2f);
        
        FilteredIMUData data = imuFilter->getFilteredData();
        float error = abs(targetHeading - data.yaw);
        if(error > 180.0f) error = 360.0f - error;
        
        maxHeadingError = fmax(maxHeadingError, error);
        
        if(!recovered && error < 5.0f) {
            recoveryTime = i * 0.1f;
            recovered = true;
        }
    }
    
    TEST_ASSERT_TRUE_MESSAGE(maxHeadingError <= 45.0f, 
        "Excessive heading error during disturbance");
    TEST_ASSERT_TRUE_MESSAGE(recoveryTime <= 15.0f, 
        "Slow recovery from disturbance");
}

void runTests() {
    UNITY_BEGIN();
    RUN_TEST(test_calm_heading_control);
    RUN_TEST(test_wave_response);
    RUN_TEST(test_wind_angle_tracking);
    RUN_TEST(test_disturbance_recovery);
    UNITY_END();
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