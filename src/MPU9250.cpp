#include "MPU9250.h"

// MPU9250 registers (common subset)
static const uint8_t WHO_AM_I       = 0x75;
static const uint8_t PWR_MGMT_1     = 0x6B;
static const uint8_t INT_PIN_CFG    = 0x37;
static const uint8_t INT_ENABLE     = 0x38;
static const uint8_t ACCEL_XOUT_H   = 0x3B;  // start of accel/gyro data
static const uint8_t GYRO_CONFIG    = 0x1B;
static const uint8_t ACCEL_CONFIG   = 0x1C;
static const uint8_t SMPLRT_DIV     = 0x19;
static const uint8_t CONFIG         = 0x1A;

// Magnetometer registers (AK8963)
static const uint8_t AK8963_ST1     = 0x02;  // data ready status bit
static const uint8_t AK8963_XOUT_L  = 0x03;  // start of mag data
static const uint8_t AK8963_CNTL    = 0x0A;  // control
static const uint8_t AK8963_ASA     = 0x10;  // sensitivity adjustment

MPU9250::MPU9250(TwoWire& wire, uint8_t address)
 : _wire(wire),
   _address(address),
   _accelRange(MPU9250AccelRange::ACCEL_RANGE_2G),
   _gyroRange(MPU9250GyroRange::GYRO_RANGE_250DPS),
   _dlpfMode(0), // default
   _srd(0),
   _accelX(0),_accelY(0),_accelZ(0),
   _gyroX(0), _gyroY(0), _gyroZ(0),
   _magX(0),  _magY(0),  _magZ(0),
   _accelScale(1.0f), _gyroScale(1.0f)
{
}

int MPU9250::begin()
{
    _wire.beginTransmission(_address);
    if(_wire.endTransmission()!=0){
        // no response on I2C
        return -1;
    }
    // Check WHO_AM_I
    uint8_t who = readByte(WHO_AM_I);
    if(who!=0x71 && who!=0x68) { 
        // 0x71 is typical for MPU9250, 0x68 is also seen 
        return -2; 
    }

    // Wake up
    writeByte(PWR_MGMT_1, 0x00); // clear sleep
    delay(10);

    // Set sample rate
    writeByte(SMPLRT_DIV, _srd);
    // set DLPF config
    writeByte(CONFIG, _dlpfMode);

    // set default accel/gyro ranges
    setAccelRange(_accelRange);
    setGyroRange(_gyroRange);

    // enable i2c master pass-through for mag
    initMag();

    // everything ok
    return 0;
}

void MPU9250::setAccelRange(MPU9250AccelRange range)
{
    _accelRange=range;
    uint8_t value=0;
    switch(range){
        case MPU9250AccelRange::ACCEL_RANGE_2G:  value=0x00; _accelScale=2.0f/32768.0f*9.81f;  break;
        case MPU9250AccelRange::ACCEL_RANGE_4G:  value=0x08; _accelScale=4.0f/32768.0f*9.81f;  break;
        case MPU9250AccelRange::ACCEL_RANGE_8G:  value=0x10; _accelScale=8.0f/32768.0f*9.81f;  break;
        case MPU9250AccelRange::ACCEL_RANGE_16G: value=0x18; _accelScale=16.0f/32768.0f*9.81f; break;
    }
    writeByte(ACCEL_CONFIG, value);
    delay(10);
}

void MPU9250::setGyroRange(MPU9250GyroRange range)
{
    _gyroRange=range;
    uint8_t value=0;
    switch(range){
        case MPU9250GyroRange::GYRO_RANGE_250DPS:   value=0x00; _gyroScale=250.0f/32768.0f*(3.14159f/180.0f); break;
        case MPU9250GyroRange::GYRO_RANGE_500DPS:   value=0x08; _gyroScale=500.0f/32768.0f*(3.14159f/180.0f); break;
        case MPU9250GyroRange::GYRO_RANGE_1000DPS:  value=0x10; _gyroScale=1000.0f/32768.0f*(3.14159f/180.0f);break;
        case MPU9250GyroRange::GYRO_RANGE_2000DPS:  value=0x18; _gyroScale=2000.0f/32768.0f*(3.14159f/180.0f);break;
    }
    writeByte(GYRO_CONFIG, value);
    delay(10);
}

void MPU9250::readSensor()
{
    // 1) read 14 bytes for accel+gyro 
    // ACCEL_XOUT_H ... GYRO_ZOUT_L
    uint8_t raw[14];
    readBytes(ACCEL_XOUT_H, 14, raw);

    int16_t ax = (int16_t)((raw[0]<<8)|raw[1]);
    int16_t ay = (int16_t)((raw[2]<<8)|raw[3]);
    int16_t az = (int16_t)((raw[4]<<8)|raw[5]);

    int16_t gx = (int16_t)((raw[8]<<8)|raw[9]);
    int16_t gy = (int16_t)((raw[10]<<8)|raw[11]);
    int16_t gz = (int16_t)((raw[12]<<8)|raw[13]);

    _accelX = ax*_accelScale;
    _accelY = ay*_accelScale;
    _accelZ = az*_accelScale;

    _gyroX  = gx*_gyroScale;
    _gyroY  = gy*_gyroScale;
    _gyroZ  = gz*_gyroScale;

    // 2) read magnetometer
    readMagData();
}

void MPU9250::initMag()
{
    // Setup the Master I2C for the mag
    // Typically, we write 0x02 to INT_PIN_CFG => BYPASS_EN
    writeByte(INT_PIN_CFG, 0x02);
    delay(10);

    // enable mag
    _wire.beginTransmission(AK8963_ADDRESS);
    _wire.write(AK8963_CNTL);
    _wire.write(0x16); // continuous measurement mode 100Hz
    _wire.endTransmission();
    delay(10);
}

void MPU9250::readMagData()
{
    // check if data ready
    _wire.beginTransmission(AK8963_ADDRESS);
    _wire.write(AK8963_ST1);
    _wire.endTransmission(false);
    _wire.requestFrom((uint8_t)AK8963_ADDRESS, (uint8_t)1);
    uint8_t st1 = _wire.read();
    if(!(st1 & 0x01)){
        // data not ready
        return;
    }

    // read 7 bytes (X L/H, Y L/H, Z L/H, ST2)
    _wire.beginTransmission(AK8963_ADDRESS);
    _wire.write(AK8963_XOUT_L);
    _wire.endTransmission(false);
    _wire.requestFrom((uint8_t)AK8963_ADDRESS, (uint8_t)7);
    if(_wire.available()<7) return;

    uint8_t magRaw[7];
    for(int i=0;i<7;i++){
        magRaw[i]=_wire.read();
    }
    int16_t mx = (int16_t)((magRaw[1]<<8) | magRaw[0]);
    int16_t my = (int16_t)((magRaw[3]<<8) | magRaw[2]);
    int16_t mz = (int16_t)((magRaw[5]<<8) | magRaw[4]);
    // ST2=magRaw[6], might contain overflow bits

    // scale: sensitivity ~0.6 uT/LSB for 16-bit at 0.15 uT/LSB, 
    // but exact depends on your setup. We'll assume ~0.6
    float magScale = 0.6f; // microtesla per LSB (example)
    _magX = mx*magScale;
    _magY = my*magScale;
    _magZ = mz*magScale;
}

uint8_t MPU9250::readByte(uint8_t reg)
{
    _wire.beginTransmission(_address);
    _wire.write(reg);
    _wire.endTransmission(false);
    _wire.requestFrom((uint8_t)_address, (uint8_t)1);
    if(_wire.available()<1) return 0;
    return _wire.read();
}

void MPU9250::readBytes(uint8_t reg, uint8_t count, uint8_t * dest)
{
    _wire.beginTransmission(_address);
    _wire.write(reg);
    _wire.endTransmission(false);
    _wire.requestFrom(_address, count);
    for(uint8_t i=0; i<count; i++){
        dest[i]=_wire.read();
    }
}

void MPU9250::writeByte(uint8_t reg, uint8_t data)
{
    _wire.beginTransmission(_address);
    _wire.write(reg);
    _wire.write(data);
    _wire.endTransmission();
}
