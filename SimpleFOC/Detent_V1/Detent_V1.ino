/**
 * 
 * Position/angle motion control example
 * Steps:
 * 1) Configure the motor and magnetic sensor  
 * 2) Run the code
 * 3) Set the target angle (in radians) from serial terminal
 * 
 */
#include <SimpleFOC.h>


// magnetic sensor AS5600
MagneticSensorI2C sensor = MagneticSensorI2C(AS5600_I2C);

// BLDC motor & driver instance
// BLDCMotor motor = BLDCMotor(pole pair number);
BLDCMotor motor = BLDCMotor(7);

// MCU：AtmelMega328PB
// Product Code : NANO SFOC B28PR36/B28P12
//BLDCDriver3PWM driver = BLDCDriver3PWM(9, 5, 6, 7);
//*********************************************************************
// MCU：STM32F103C8T6
// Product Code : NANO SFOC S28PR36/S28PR12
BLDCDriver3PWM driver = BLDCDriver3PWM(PA6, PA7, PB0, PB12);


// angle set point variable
float target_angle = 0;
const int virtualdetent = 18;

// instantiate the commander
Commander command = Commander(Serial);
void doTarget(char* cmd) {
  command.scalar(&target_angle, cmd);
}

void setup() {

  // initialise magnetic sensor hardware
  sensor.init();
  // link the motor to the sensor
  motor.linkSensor(&sensor);

  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 12;
  driver.init();
  // link the motor and the driver
  motor.linkDriver(&driver);

  // choose FOC modulation (optional)
  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;

  // set motion control loop to be used
  motor.controller = MotionControlType::angle;

  // contoller configuration
  // default parameters in defaults.h

  // velocity PI controller parameters
  motor.PID_velocity.P = 0.05;
  motor.PID_velocity.I = 2;
  motor.PID_velocity.D = 0.001;
  // maximal voltage to be set to the motor
  motor.voltage_limit = 6;

  // velocity low pass filtering time constant
  // the lower the less filtered
  motor.LPF_velocity.Tf = 0.01;

  // angle P controller
  motor.P_angle.P = 10;
  // maximal velocity of the position control
  motor.velocity_limit = 20;

  // use monitoring with serial
  Serial.begin(115200);
  // comment out if not needed
  motor.useMonitoring(Serial);


  // initialize motor
  motor.init();
  // align sensor and start FOC
  motor.initFOC();

  // add target command T
  command.add('T', doTarget, "target angle");

  // Serial.println(F("Motor ready."));
  // Serial.println(F("Set the target angle using serial terminal:"));
  _delay(1000);
}


void loop() {

  // main FOC algorithm function
  // the faster you run this function the better
  // Arduino UNO loop  ~1kHz
  // Bluepill loop ~10kHz
  motor.loopFOC();

  // Motion control function
  // velocity, position or voltage (defined in motor.controller)
  // this function can be run at much lower frequency than loopFOC() function
  // You can also use motor.move() and set the motor.target in the code
  // Serial.println("Target Angle");
  // Serial.println(target_angle);
  target_angle = (round(abs(fmod(motor.shaft_angle, (2 * PI))) / (2 * PI / virtualdetent))) * (2 * PI / virtualdetent);

  motor.move(target_angle);
  Serial.println(motor.shaft_angle * 180 / PI);


  // function intended to be used with serial plotter to monitor motor variables
  // significantly slowing the execution down!!!!
  // motor.monitor();

  // user communication
  command.run();
  delay(10);
}