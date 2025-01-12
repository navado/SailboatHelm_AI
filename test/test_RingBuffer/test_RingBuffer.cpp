#include <unity.h>
#include <RingBuffer.h>

static RingBuffer<int, 3> ring;

void setUp(){ 
    // ...
}
void tearDown(){ 
    // ...
}

void test_ring_usage() {
    // forcibly add samples
    for(int i=0; i<3; i++){
        ring.push(i);
    }
    int count=0;
    for(int i=0; i<3; i++){
        int val;
        TEST_ASSERT_TRUE(ring.pop(val));
        TEST_ASSERT_EQUAL_INT(i, val);
        count++;
    }
}

void test_ring_overflow() {
    // forcibly add samples
    for(int i=0; i<5; i++){
        ring.push(i);
    }
    for (int i=0; i<3; i++){
        int val;
        TEST_ASSERT_TRUE(ring.pop(val));
        TEST_ASSERT_EQUAL_INT(i+2, val);
    }
}

#ifdef ARDUINO
void setup(){
    UNITY_BEGIN();
    RUN_TEST(test_ring_usage);
    RUN_TEST(test_ring_overflow);
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
