
// Requires the Adafruit_Motorshield v2 library 
//   https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library
// And AccelStepper with AFMotor support 
//   https://github.com/adafruit/AccelStepper
// Comes from "Accel_MultiStepper_CT.ino"

#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include <Adafruit_PWMServoDriver.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMSBot(0x60); // No jumpers
Adafruit_StepperMotor *myStepperX = AFMSBot.getStepper(200,1);  //First value is steps/rev of motor, Second value-> 1= motor M1/M2, 2= motor M3/M4
Adafruit_StepperMotor *myStepperY = AFMSBot.getStepper(200,2);  //First value is steps/rev of motor, Second value-> 1= motor M1/M2, 2= motor M3/M4

// Stage from Amazon - https://www.amazon.com/Befenybay-Ballscrew-SFU1605-Stepper-Actuator/dp/B085STFFM7/ref=pd_ci_mcx_di_int_sccai_cn_d_sccl_3_6/132-9613518-1192923?pd_rd_w=UVFxE&content-id=amzn1.sym.751acc83-5c05-42d0-a15e-303622651e1e&pf_rd_p=751acc83-5c05-42d0-a15e-303622651e1e&pf_rd_r=CFM3NDRJ0KF907C9V476&pd_rd_wg=4KSXZ&pd_rd_r=68ea85c7-c0c8-4095-84dd-2c8a2b66d7ae&pd_rd_i=B07W89HSY8&th=1 
// 5mm/rev (leadscrew pitch)
// Well spacing = 9mm which equals 360 steps

String command;
int xPosition = 0;   //Variable to define X postion.
int yPosition = 0;   //Variable to define Y postion.
int wellPosx = 2000;   //Input the steps in X to center on first well.
int wellPosy = 2000;   //Input the steps in Y to center on first well.
int wellCount = 1;     //Counts number of wells to ensure we don't exceed 96 and determines when to raster to next column.
int oneWellstep = 360;   // steps per one well move.
int ruptPinx = 0;    //0= motor M1/M2, 1= motor M3/M4
int ruptPiny = 1;    //0= motor M1/M2, 1= motor M3/M4
int requestedPulses;
String axis;  //Character defining which axis requested
boolean sensorFailx;
boolean sensorFaily;

// you can change these to DOUBLE or INTERLEAVE or MICROSTEP!
// wrappers for the motor!
void forwardstepX() {  
  myStepperX->onestep(FORWARD, DOUBLE);
}
void backwardstepX() {  
  myStepperX->onestep(BACKWARD, DOUBLE);
}
void forwardstepY() {  
  myStepperX->onestep(FORWARD, DOUBLE);
}
void backwardstepY() {  
  myStepperX->onestep(BACKWARD, DOUBLE);
}
// Now we'll wrap the steppers in an AccelStepper object
AccelStepper stepperX(forwardstepX, backwardstepX);  //flip variables to change default motor direction
AccelStepper stepperY(forwardstepY, backwardstepY);  //flip variables to change default motor direction
void setup()
{  
  AFMSBot.begin();    //Starts motor shield
// TWBR = ((F_CPU /400000l) - 16) / 2; // Change the i2c clock to 400KHz
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  Serial.println(F("Arduino awake"));
  Serial.println(F("Serial parse sketch to input motor control parameters"));
  Serial.println(F("Ex. abx12345 or abx-12345"));
  Serial.println(F("Type (help) for list of commands"));

  stepperX.setAcceleration(1000); 
  stepperX.setMaxSpeed(1000);   //For Haydon-Kerk motor E25541-05 max speed approx 400. leadscrew .001"/step

  stepperY.setAcceleration(1000); 
  stepperY.setMaxSpeed(1000);   //For Haydon-Kerk motor E25541-05 max speed approx 400. leadscrew .001"/step

// Initially home both motors and move to well 1
axis="x";
Home();
delay (1000);
requestedPulses = wellPosx;  // Moves to calibrated position for X 
absoluteMove();
delay (1000);
//axis="y";
//Serial.println(axis);
//Home();
//delay (1000);
//requestedPulses = wellPosy;  // Moves to calibrated position for y 
//absoluteMove();
//delay (1000);
}

void loop()
{
//  sensorFailx = false;
//  sensorFaily = false;
  // reply only when you receive data:
  if (Serial.available())
  {
     char c = Serial.read();
     if(c == '\n')
     {
        parseCommand(command);
         command = "";
      }
       else
       {
         command += c;
        }
  }
}
void parseCommand(String com)  //Takes in command and parses info.  ex. abx125 or abx-456. Displays error of bad command
{
  String part1;  // 3 characters
  String part2;  //interger
  part1 = com.substring(0,2);
  axis = com.substring(2,3);
  part2 = com.substring(3, '\n');
  requestedPulses = (part2.toInt());   //Loads integer into "requestedPulses"

if(part1.equalsIgnoreCase("pr"))   // Implements protocol
  {
    protocol();
  }
else if(part1.equalsIgnoreCase("mo"))   //Request move to next well
  {
    Serial.println("Asking nextWell void");
    nextWell();
  }
else if(part1.equalsIgnoreCase("ab"))   //Makes and absolute move for x axis
  {
    absoluteMove();
  }
else if(part1.equalsIgnoreCase("re"))    //Makes relative move for x axis
  {
    relativeMove();
  }
else if(part1.equalsIgnoreCase("ho"))   //Homes the x axis
  {
    Home();
  }
else if(part1.equalsIgnoreCase("po"))   //Sets current position to desired value
  {
    setPosition();
  }
  else if(part1.equalsIgnoreCase("he"))   //Displays available commands
  {
    Serial.println(F("protocol                           Implements desired protocol"));
    Serial.println(F("mo                                 Request to move to next well"));
    Serial.println(F("ho(x, y or z)                      Homes motor of desired axis ex. hoz"));
    Serial.println(F("ab(x, y, z)(integer)               Absolute move (positive only) ex. abx123"));
    Serial.println(F("re(x, y, z)(integer)               Relative move (pos or neg)from current position ex. rez-123"));
    Serial.println(F("po(x, y, z)(integer)               Sets current position to user input"));
    Serial.println(F("help Provides list of commands available"));
  }
  else
  {
    Serial.println(F("Command not recongnized"));
    Serial.println(part1);
  }
}

void nextWell()
{
  Serial.println("In nextWell void");
  if (wellCount = 9 || 17 || 25 || 33 || 41 || 49 || 57 || 65 || 73 || 81 || 97)  // Checks for if move to next column required.
  {
    axis = "x";
    requestedPulses = -360;
    relativeMove();
    axis = "x";
    requestedPulses = oneWellstep * 7;
    relativeMove();
  }
  else
  {
    axis = "x";
    requestedPulses = -360;
    relativeMove();
  }
}

void protocol()
{
  int numberScan = 10;  //Total number of back/forth scans for test
  int totalNumberScan = numberScan;
  axis="x";
  Home();
  requestedPulses = 50;  // Starting position of scan
  while(numberScan != 0 && sensorFailx == false)
  {
    requestedPulses = 1010;  // 960 pulses = 24mm (add starting position)
    absoluteMove();
    requestedPulses = 50;  // Starting position of scan
    absoluteMove();
    numberScan--;  // Decrease by 1 for next move if needed
    Serial.print(totalNumberScan - numberScan);
    Serial.print(" of ");
    Serial.println(totalNumberScan);
  }
  Serial.println(F("Protocol complete"));
}
void absoluteMove()
{
  if (axis == "x")
  {
    stepperX.setSpeed(800); 
    stepperX.setAcceleration(1000);
    Serial.print(F("Absolute X motor move = "));
    Serial.println(requestedPulses);
    xPosition = requestedPulses;
    if (xPosition < 20000  && xPosition > -20000)  // Checks for commands greater than travel range
    {
      stepperX.moveTo(requestedPulses);
      stepperX.runToPosition();
      myStepperX->release();
      Serial.print (xPosition);
      Serial.println (F(", "));
    }
    else
      {
        Serial.println(F("Position request exceeds travel. Command aborted"));
      }
  }
  else if (axis == "y")
    {
    stepperY.setSpeed(800); 
    stepperY.setAcceleration(1000);
    Serial.print(F("Absolute Y motor move = "));
    Serial.println(requestedPulses);
    yPosition = requestedPulses;
    if (yPosition < 20000  && yPosition > -20000)  // Checks for commands greater than travel range
    {
      stepperY.moveTo(requestedPulses);
      stepperY.runToPosition();
      myStepperY->release();
      Serial.print (yPosition);
      Serial.println (F(", "));
    }
    else
      {
        Serial.println(F("Position request exceeds travel. Command aborted"));
      }
    }
}
void relativeMove()
{
    if (axis == "x")
    {
      stepperX.setSpeed(800); 
      stepperX.setAcceleration(1000);
      Serial.print(F("Relative X motor move = "));
      Serial.println(requestedPulses);
      xPosition = stepperX.currentPosition() + requestedPulses;
      if (xPosition < 20000  && xPosition > -20000)  // Checks for commands greater than travel range
      {
        stepperX.moveTo(xPosition);
        stepperX.runToPosition();
        myStepperX->release();
        Serial.print (xPosition);
        Serial.println (F(", "));
      }
      else
      {
        Serial.println(F("Position request exceeds travel. Command aborted"));
      }
    }
   else if (axis == "y")
   {
    stepperY.setSpeed(800); 
    stepperY.setAcceleration(1000);
    Serial.print(F("Relative Y motor move = "));
      Serial.println(requestedPulses);
      yPosition = stepperY.currentPosition() + requestedPulses;
      if (yPosition < 20000  && yPosition > -20000)  // Checks for commands greater than travel range
      {
        stepperY.moveTo(yPosition);
        stepperY.runToPosition();
        myStepperY->release();
        Serial.print (yPosition);
        Serial.println (F(", "));
      }
      else
      {
        Serial.println(F("Position request exceeds travel. Command aborted"));
      }
   }
}

void Home()
{ 
  if (axis == "x")
  {
     bool sensorFailx = false;
     Serial.println("Homing X");
     stepperX.setCurrentPosition(0);
   if(analogRead(ruptPinx) < 600)   // if flag at home, move off and continue to home. Identify if opto not working.
    {
     stepperX.moveTo(1000);  // Move off flag 100 steps
     stepperX.runToPosition();  // Start moving the stepper
     if (analogRead(ruptPinx) < 600)
     Serial.println (F("Sensor not functional. Homing failed"));
     sensorFailx = true;
   }
   delay(1000);
   long initial_homingx=6000;  // Used to Home Stepper at startup. Must be away from home
   stepperX.setCurrentPosition(initial_homingx);
  while(analogRead(ruptPinx) > 600 && sensorFailx == false) {
    stepperX.moveTo(initial_homingx);  // Set the position to move to
    initial_homingx--;  // Decrease by 1 for next move if needed
    stepperX.run();  // Start moving the stepper
   }
    int backStep = 100;
    stepperX.setCurrentPosition(0);
    stepperX.moveTo(-10);
    stepperX.runToPosition();
    stepperX.setSpeed(1); 
    stepperX.setAcceleration(1);
  while(analogRead(ruptPinx) < 600 && sensorFailx == false) {
    stepperX.moveTo(backStep);  // Move into flag 10 steps
    backStep++;  // Increase by 1 for next move if needed
    stepperX.runSpeed();  // Start moving the stepper
  }
  Serial.println("X homed");
  Serial.print (xPosition);
  Serial.println (F(", "));
  stepperX.setCurrentPosition(0);
  xPosition = stepperX.currentPosition();
  myStepperX->release(); 
  stepperX.setSpeed(800); 
  stepperX.setAcceleration(200);
//  sensorFailx = false;
  }
  else if (axis == "y")
  {
     bool sensorFaily = false;
     Serial.println("Homing Y");
     stepperY.setCurrentPosition(0);
   if(analogRead(ruptPiny) < 600)   // if flag at home, move off and continue to home. Identify if opto not working.
    {
     stepperY.moveTo(100);  // Move off flag 100 steps
     stepperY.runToPosition();  // Start moving the stepper
     if (analogRead(ruptPiny) < 600)
     Serial.println (F("Sensor not functional. Homing failed"));
     sensorFaily = true;
   }
   delay(1000);
   long initial_homingy=6000;  // Used to Home Stepper at startup. Must be away from home
   stepperY.setCurrentPosition(initial_homingy);
  while(analogRead(ruptPiny) > 600 && sensorFaily == false) {
    stepperY.moveTo(initial_homingy);  // Set the position to move to
    initial_homingy--;  // Decrease by 1 for next move if needed
    stepperY.run();  // Start moving the stepper
   }
    int backStep = 100;
    stepperY.setCurrentPosition(0);
    stepperY.moveTo(-10);
    stepperY.runToPosition();
    stepperY.setSpeed(1); 
    stepperY.setAcceleration(1);
  while(analogRead(ruptPiny) < 600 && sensorFaily == false) {
    stepperY.moveTo(backStep);  // Move into flag 10 steps
    backStep++;  // Increase by 1 for next move if needed
    stepperY.runSpeed();  // Start moving the stepper
  }
  Serial.println("Y homed");
  Serial.print (yPosition);
  Serial.println (F(", "));
  stepperY.setCurrentPosition(0);
  yPosition = stepperY.currentPosition();
  myStepperY->release(); 
  stepperY.setSpeed(800); 
  stepperY.setAcceleration(200);
  sensorFaily = false;
  }
}
void setPosition()
{
    if (axis == "x")
    {
      Serial.print(F("Current X = "));
      Serial.println(requestedPulses);
      xPosition = requestedPulses;   //Sets X position to user input
    }
   else if (axis == "y")
   {
    Serial.print(F("Relative Y motor move = "));
      Serial.println(requestedPulses);
      yPosition = stepperY.currentPosition() + requestedPulses;
      if (yPosition < 2001  && yPosition > -20000)  // Checks for commands greater than travel range
      {
        stepperY.moveTo(yPosition);
        stepperY.runToPosition();
        myStepperY->release();
        Serial.print (yPosition);
        Serial.println (F(", "));
      }
      else
      {
        Serial.println(F("Position request exceeds travel. Command aborted"));
      }
   }
}
