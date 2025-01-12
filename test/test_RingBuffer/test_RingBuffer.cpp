#include <unity.h>
#include "MyIMUProvider.h"

static MyIMUProvider imu;

void setUp(){ 
    // ...
}
void tearDown(){ 
    // ...
}

void test_ring_usage() {
    // forcibly add samples
    for(int i=0; i<3; i++){
        imu.readSensorInISR();
    }
    IMUData d;
    int count=0;
    while(imu.getIMUData(d)){
        count++;
    }
    TEST_ASSERT_EQUAL(3, count);
}

#ifdef ARDUINO
void setup(){
    UNITY_BEGIN();
    RUN_TEST(test_ring_usage);
    UNITY_END();
}
void loop(){}
#else
int main(){
    UNITY_BEGIN();
    RUN_TEST(test_ring_usage);
    return UNITY_END();
}
#endif
