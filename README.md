![Boat Autopilot Programming](docs/img/AIAutoPilot.png)

# What is this?

This code in this version created in attempt to evaluate ability of ChatGPT to create useful code on interesting topic. I selected boat autopipot on embedded system as an example to learn capabilities and illustrate them.

## The goal

Is to create yet another opensource autopilot but heavily assisted by AI. In the roadmap 

## Current state

As for the date of this commit the code is compiling. I'm intending to make it work, but currently don't have enough parts to properly test it. When order arrives, I'll get to it. 

## How to contribute
You can create PRs, test, improve, discuss. I will appreciate all the feedback

## Roadmap

 - [X] Create basic code and compile
 - [X] Add unit tests that run with ` pio test -e test`. Make tests run.
 - [X] Make tests sane
 - [ ] Define electronic connections and protocols
 - [ ] Make the initial theoretically working version of the code
 - [X] Basic UI with tests
 - [ ] Select proper screen and its connections, preferably 4 inch screen with buttons module, update pins and connections
 - [ ] Assemble and test UI
 - [ ] RX data from NMEA0183 over wifi
 - [ ] TX commands using NMEA0183 over wifi (define $P protocol)
 - [ ] Add command only mode without the stering mechanics and vise versa, use external course computer
 - [ ] Rewrite everything in Rust
 - [ ] Virtualization of all major components
 - [ ] TBD

## Class diagram 

```mermaid
classDiagram

%% ================== Interfaces ==================
class IIMUProvider {
  <<interface>>
  +getIMUData(outData : IMUData) bool
}
class ITimeProvider {
  <<interface>>
  +getMillis() uint64
}
class IInputDevice {
  <<interface>>
  +isPressed(btn : ButtonId) bool
}

%% ================== Autopilot ==================
class AutoSteeringController {
  -_mode : AutoSteeringMode
  -_desiredHeading : float
  -_desiredCourse : float
  -_desiredWindAngle : float
  -_rudderAngle : float
  -_kP, _kI, _kD : float
  -_integral, _lastError : float
  +setMode(mode : AutoSteeringMode, param : float)
  +update(dt : float)
  +getRudderAngle() float
}

%% ================== IMU Filter ==================
class IMUFilterAndCalibration {
  -_imu : IIMUProvider& 
  -_time : ITimeProvider&
  -_calibrating : bool
  -_pitch, _roll, _yaw : float
  -_lastUpdate : uint64
  +startCalibration()
  +doCalibrationStep()
  +update()
  +getFilteredData() FilteredIMUData
}

%% ================== IMU Provider ==================
class MyIMUProvider {
  -_i2cAddr : uint8
  -_drPin : int
  -_dummyVal : float
  -_ring : RingBuffer<IMUData, N>
  +begin() bool
  +getIMUData(outData : IMUData) bool
  +readSensorInISR()
  +onImuDataReady()
}
MyIMUProvider --|> IIMUProvider : implements

%% ================== Ring Buffer ==================
class "RingBuffer<T,N>" {
  -_buffer : T[N]
  -_head, _tail : size_t
  -_count : size_t
  +push(item : T) bool
  +pop(out : T) bool
  +isFull() bool
  +isEmpty() bool
  +size() size_t
}

MyIMUProvider o-- "RingBuffer<IMUData,N>" : has

%% ================== UI Model/Controller/View ==================
class UIModel {
  -_state : UIState
  +setAutoMode(...)
  +setSteeringMode(...)
  +setHeadingSetpoint(...)
  +incrementSetpointSmall()
  ...
}
class UIController {
  -_model : UIModel&
  -_autoSteer : AutoSteeringController&
  -_input : IInputDevice&
  +update()
  +cycleSteeringMode()
  +updateAutoSteerSetpoint()
}
class UIView {
  +begin() bool
  +render(model : UIModel)
}

UIController --> UIModel : "has reference"
UIController --> AutoSteeringController : "has reference"
UIController --> IInputDevice : "uses"

UIView --> UIModel : "render(...) reads"

%% ================== Additional Relations ==================
IMUFilterAndCalibration --> IIMUProvider : "owns reference"
IMUFilterAndCalibration --> ITimeProvider : "owns reference"
```