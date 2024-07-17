
// And AccelStepper with AFMotor support
//   https://github.com/adafruit/AccelStepper
// Comes from "Accel_MultiStepper_CT.ino"

#include <Wire.h>
#include <AccelStepper.h>

// Stage from Amazon - https://www.amazon.com/Befenybay-Ballscrew-SFU1605-Stepper-Actuator/dp/B085STFFM7/ref=pd_ci_mcx_di_int_sccai_cn_d_sccl_3_6/132-9613518-1192923?pd_rd_w=UVFxE&content-id=amzn1.sym.751acc83-5c05-42d0-a15e-303622651e1e&pf_rd_p=751acc83-5c05-42d0-a15e-303622651e1e&pf_rd_r=CFM3NDRJ0KF907C9V476&pd_rd_wg=4KSXZ&pd_rd_r=68ea85c7-c0c8-4095-84dd-2c8a2b66d7ae&pd_rd_i=B07W89HSY8&th=1
// 5mm/rev (leadscrew pitch)
// 1600steps/rev (TMC2209 defualts to 8ustep)
// Well spacing = 9mm which equals 2880 steps
// NEMA 17 motors, 12V, 1.5A, 200steps/rev

String command;
const int pinEnAx = 10;
const int pinEnAy = 7;
int numMicrosteps = 8;
int numSteps = 1000;
const int numRev = 1;
const int stepsPerRevolution = 200;
int yDirection = 1;
long xPosition = 0;       //Variable to define X postion.
long yPosition = 0;       //Variable to define Y postion.
int wellPosx = 5200;     //Input the steps in X to center on first well.
int wellPosy = 2950;     //Input the steps in Y to center on first well.
int wellCount = 1;       //Counts number of wells to ensure we don't exceed 96 and determines when to raster to next column.
int oneWellstep = 2880;  // steps per one well move.
int ruptPinx = 0;        //  Analog pin for monitoring X opto
int ruptPiny = 1;        //  Analog pin for monitoring Y opto
long requestedPulses;
String axis;              //Character defining which axis requested
String status = "READY";  // String describing system status <READY|MOVING|ERROR>
boolean sensorFailx;
boolean sensorFaily;

// Now we'll wrap the steppers in an AccelStepper object
AccelStepper stepperX(1, 9, 8);  // (Typeof driver: with 2 pins, STEP, DIR)
AccelStepper stepperY(1, 6, 5);  // (Typeof driver: with 2 pins, STEP, DIR)
void setup() {
  pinMode(pinEnAx, OUTPUT);
  digitalWrite(pinEnAx, LOW);  //enables motor driver
  pinMode(pinEnAy, OUTPUT);
  digitalWrite(pinEnAy, LOW);  //enables motor driver

  Serial.begin(9600);  // opens serial port, sets data rate to 9600 bps
  Serial.println(F("Arduino awake"));
  Serial.println(F("File: SCC_96_Well_Fixture_TMC2209_CE_R2"));
  Serial.println(F("Serial parse sketch to input motor control parameters"));
  Serial.println(F("Ex. abx12345 or abx-12345"));
  Serial.println(F("Type (help) for list of commands:"));
  Serial.println(F(" "));

  stepperX.setAcceleration(3000);
  stepperX.setMaxSpeed(3000);

  stepperY.setAcceleration(3000);
  stepperY.setMaxSpeed(3000);

  stepperX.setCurrentPosition(0);  // Set the current position to 0 steps
  stepperY.setCurrentPosition(0);  // Set the current position to 0 steps
  delay(1000);
}

void loop() {
  //  sensorFailx = false;
  //  sensorFaily = false;
  // reply only when you receive data:
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      parseCommand(command);
      command = "";
    } else {
      command += c;
    }
  }
}
void parseCommand(String com)  //Takes in command and parses info.  ex. abx125 or abx-456. Displays error of bad command
{
  String part1;  // 3 characters
  String part2;  //interger
  part1 = com.substring(0, 2);
  axis = com.substring(2, 3);
  part2 = com.substring(3, '\n');
  requestedPulses = (part2.toInt());  //Loads integer into "requestedPulses"

  /////  Go to first well
  if (part1.equalsIgnoreCase("fw"))  // Implements protocol
  {
    Serial.println("FW:ACCEPTED");
    firstWell();
    Serial.println("FW:COMPLETE");
  /////  Request axis position
  } else if (part1.equalsIgnoreCase("ps"))  // Request axis position
  {
    if (com.length() < 3) {
      Serial.println("Get Axis Position - not enough arguments");
      Serial.println(F("ab(x, y, z)(integer)               get axis position ex. psx"));
      Serial.println("PS:REJECTED");
    } else if (!((axis == "x") || (axis == "y") || (axis == "z"))) {
      Serial.println(F("Get Axis Position - axis specified must be x y or z"));
      Serial.println(F("ab(x, y, z)(integer)               get axis position ex. psx"));
      Serial.println("PS:REJECTED");
    } else if (axis == "x") {
      String xPos = String(stepperX.currentPosition());
      Serial.println("PS:ACCEPTED:" + xPos);
    } else if (axis == "y") {
      String yPos = String(stepperY.currentPosition());
      Serial.println("PS:ACCEPTED:" + yPos);
    }  //else if (axis=="z") {
    Serial.println("PS:COMPLETE");

  /////  Request move to next well

  } else if (part1.equalsIgnoreCase("mo"))
  {
    Serial.println("Asking nextWell void");
    Serial.println("MO:ACCEPTED");
    nextWell();
    Serial.println("MO:COMPLETE");

  /////  Return current well index

  } else if (part1.equalsIgnoreCase("cw"))
  {
    String wellString = String(wellCount);
    Serial.println("CW:ACCEPTED:" + wellString);
    Serial.println("CW:COMPLETE");

  /////  Makes and absolute move for x axis

  } else if (part1.equalsIgnoreCase("ab")) 
  {
    // Validate command here
    if (com.length() < 4) {
      Serial.println("Absolute move - not enough arguments");
      Serial.println(F("ab(x, y, z)(integer)               Absolute move (positive only) ex. abx123"));
      Serial.println("AB:REJECTED");
    } else if (requestedPulses >= 20000 || requestedPulses <= -20000) {
      Serial.println("Absolute move - requested move out of bounds");
      Serial.println(F("ab(x, y, z)(integer)               Absolute move (positive only) ex. abx123"));
      Serial.println("AB:REJECTED");
    } else if (!((axis == "x") || (axis == "y") || (axis == "z"))) {
      Serial.println(F("Absolute move - axis specified must be x y or z"));
      Serial.println(F("ab(x, y, z)(integer)               Absolute move (positive only) ex. abx123"));
      Serial.println("AB:REJECTED");
    } else {
      Serial.println("AB:ACCEPTED");  // Might want to add checking for additional arguments here
      absoluteMove();
    }
    Serial.println("AB:COMPLETE");

  /////  Makes and relative move for x axis

  } else if (part1.equalsIgnoreCase("re"))  
  {
    // Validate command here
    if (com.length() < 4) {
      Serial.println("Relative move - not enough arguments");
      Serial.println(F("re(x, y, z)(integer)               Relative move (pos or neg)from current position ex. rez-123"));
      Serial.println("RE:REJECTED");
    } else if (axis == "x" && (requestedPulses + stepperX.currentPosition() >= 20000 || requestedPulses + stepperX.currentPosition() <= -20000)) {
      Serial.println("Relative move - requested move out of bounds");
      Serial.println(F("re(x, y, z)(integer)               Relative move (pos or neg)from current position ex. rez-123"));
      Serial.println("RE:REJECTED");
    } else if (axis == "y" && (requestedPulses + stepperY.currentPosition() >= 20000 || requestedPulses + stepperY.currentPosition() <= -20000)) {
      Serial.println("Relative move - requested move out of bounds");
      Serial.println(F("re(x, y, z)(integer)               Relative move (pos or neg)from current position ex. rez-123"));
      Serial.println("RE:REJECTED");
    } else {
      Serial.println("RE:ACCEPTED");  // Might want to add checking for additional arguments here
      relativeMove();
    }
    Serial.println("RE:COMPLETE");

  /////  Homes the x axis

  } else if (part1.equalsIgnoreCase("ho")) 
  {
    Serial.println("HO:ACCEPTED");
    Home();
    Serial.println("HO:COMPLETE");

  /////  Sets current position to desired value

  } else if (part1.equalsIgnoreCase("po"))  
  {
    Serial.println("PO:ACCEPTED");  // Might want to add checking for additional arguments here
    setPosition();
    Serial.println("PO:COMPLETE");

  /////  Motor test?

  } else if (part1.equalsIgnoreCase("tm"))
  {
    Serial.println("TM:ACCEPTED");  // Validation?
    testMotor();
    Serial.println("TM:COMPLETE");

  /////  Displays available commands

  } else if (part1.equalsIgnoreCase("he"))
  {
    Serial.println("HE:ACCEPTED");
    Serial.println(F("fw                                 Moves to first well"));
    Serial.println(F("mo                                 Request to move to next well"));
    Serial.println(F("ho(x, y or z)                      Homes motor of desired axis ex. hoz"));
    Serial.println(F("ab(x, y, z)(integer)               Absolute move (positive only) ex. abx123"));
    Serial.println(F("re(x, y, z)(integer)               Relative move (pos or neg)from current position ex. rez-123"));
    Serial.println(F("po(x, y, z)(integer)               Sets current position by user input"));
    Serial.println(F("tm(x, y, z)(integer)               Cycle motors back and forth"));
    Serial.println(F("help Provides list of commands available"));
    Serial.println("HE:COMPLETE");

  /////  Report Status

  } else if (part1.equalsIgnoreCase("st"))
  {
    String statusString = status;
    Serial.println("ST:ACCEPTED:" + statusString);
    Serial.println("ST:COMPLETE");
  } else {
    Serial.println(F("Command not recongnized"));
    Serial.println(part1);
  }
}

void nextWell() {
  Serial.print(F("Current well # "));
  Serial.println(wellCount);
  if (wellCount <= 96)   // Checks to see if exceed 96 wells.
  {
    if (wellCount % 8 == 0)  // Checks for if move to next column required.
    {
      axis = "x";
      stepperX.setMaxSpeed(3000);
      stepperX.setAcceleration(3000);
      requestedPulses = oneWellstep;
      relativeMove();
      yDirection = yDirection * -1;
      wellCount++;
      Serial.print(F("Well count "));
      Serial.println(wellCount);
    } 
    else {
      axis = "y";
      stepperY.setSpeed(3000);
      stepperY.setAcceleration(3000);
      requestedPulses = oneWellstep * yDirection;
      relativeMove();
      wellCount++;
      Serial.print(F("Well count "));
      Serial.println(wellCount);
    }
  }
  else{
    Serial.print(F("Exceeded max number of wells"));
    axis = "x";
    Home();
    axis = "y";
    Home();
  }
}

void firstWell() {
  yDirection = 1
  stepperX.setMaxSpeed(3000);
  stepperX.setAcceleration(3000);
  Serial.println(F("Moving to first well"));
  requestedPulses = wellPosx;
  stepperX.moveTo(requestedPulses);
  stepperX.runToPosition();

  stepperY.setMaxSpeed(3000);
  stepperY.setAcceleration(3000);
  requestedPulses = wellPosy;
  stepperY.moveTo(requestedPulses);
  stepperY.runToPosition();
  wellCount = 1;
  Serial.println(F("At first well"));
  Serial.println(F("Well count "));
  Serial.println(wellCount);
}

void absoluteMove() {
  if (axis == "x") {
    stepperX.setMaxSpeed(3000);
    stepperX.setAcceleration(3000);
    Serial.print(F("Absolute X motor move = "));
    Serial.println(requestedPulses);
    xPosition = requestedPulses;
    if (xPosition < 200000 && xPosition > -200000)  // Checks for commands greater than travel range
    {
      stepperX.moveTo(requestedPulses);
      stepperX.runToPosition();
      Serial.print(xPosition);
      Serial.println(F(", "));
    } else {
      Serial.println(F("Position request exceeds travel. Command aborted"));
    }
  } else if (axis == "y") {
    stepperY.setMaxSpeed(3000);
    stepperY.setAcceleration(3000);
    Serial.print(F("Absolute Y motor move = "));
    Serial.println(requestedPulses);
    yPosition = requestedPulses;
    if (yPosition < 200000 && yPosition > -200000)  // Checks for commands greater than travel range
    {
      stepperY.moveTo(requestedPulses);
      stepperY.runToPosition();
      Serial.print(yPosition);
      Serial.println(F(", "));
    } else {
      Serial.println(F("Position request exceeds travel. Command aborted"));
    }
  }
}
void relativeMove() {
  if (axis == "x") {
//    stepperX.setSpeed(500);
//    stepperX.setAcceleration(1000);
    Serial.print(F("Relative X motor move = "));
    Serial.println(requestedPulses);
    xPosition = stepperX.currentPosition() + requestedPulses;
    if (xPosition < 2000000 && xPosition > -2000000)  // Checks for commands greater than travel range
    {
      stepperX.setAcceleration(3000);
      stepperX.setMaxSpeed(3000);
      stepperX.moveTo(xPosition);
      stepperX.runToPosition();  // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position
      Serial.print(xPosition);
      Serial.println(F(", "));
    } else {
      Serial.println(F("Position request exceeds travel. Command aborted"));
    }
  } else if (axis == "y") {
    stepperY.setMaxSpeed(3000);
    stepperY.setAcceleration(3000);
    Serial.print(F("Relative Y motor move = "));
    Serial.println(requestedPulses);
    yPosition = stepperY.currentPosition() + requestedPulses;
    if (yPosition < 2000000 && yPosition > -2000000)  // Checks for commands greater than travel range
    {
      stepperY.moveTo(yPosition);
      stepperY.runToPosition();  // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position
      Serial.print(yPosition);
      Serial.println(F(", "));
    } else {
      Serial.println(F("Position request exceeds travel. Command aborted"));
    }
  }
}

void Home() {
  if (axis == "x") {
    bool sensorFailx = false;
    Serial.println("Homing X");
    stepperX.setCurrentPosition(0);
    if (analogRead(ruptPinx) < 600)  // if flag at home, move off and continue to home. Identify if opto not working.
    {
      stepperX.moveTo(1000);     // Move off flag 100 steps
      stepperX.runToPosition();  // Start moving the stepper
      if (analogRead(ruptPinx) < 600)
        Serial.println(F("Sensor not functional. Homing failed"));
      sensorFailx = true;
    }
    delay(1000);
    long initial_homingx = 6000;  // Used to Home Stepper at startup. Must be away from home
    stepperX.setCurrentPosition(initial_homingx);
    while (analogRead(ruptPinx) > 600 && sensorFailx == false) {
      stepperX.moveTo(initial_homingx);  // Set the position to move to
      initial_homingx--;                 // Decrease by 1 for next move if needed
      stepperX.run();                    // Start moving the stepper
    }
    int backStep = 100;
    stepperX.setCurrentPosition(0);
    stepperX.moveTo(-10);
    stepperX.runToPosition();
    stepperX.setSpeed(1);
    stepperX.setAcceleration(1);
    while (analogRead(ruptPinx) < 600 && sensorFailx == false) {
      stepperX.moveTo(backStep);  // Move into flag 10 steps
      backStep++;                 // Increase by 1 for next move if needed
      stepperX.runSpeed();        // Start moving the stepper
    }
    Serial.println("X homed");
    Serial.print(xPosition);
    Serial.println(F(", "));
    stepperX.setCurrentPosition(0);
    xPosition = stepperX.currentPosition();
    stepperX.setSpeed(800);
    stepperX.setAcceleration(200);
    //  sensorFailx = false;
  } else if (axis == "y") {
    bool sensorFaily = false;
    Serial.println("Homing Y");
    stepperY.setCurrentPosition(0);
    if (analogRead(ruptPiny) < 600)  // if flag at home, move off and continue to home. Identify if opto not working.
    {
      stepperY.moveTo(100);      // Move off flag 100 steps
      stepperY.runToPosition();  // Start moving the stepper
      if (analogRead(ruptPiny) < 600)
        Serial.println(F("Sensor not functional. Homing failed"));
      sensorFaily = true;
    }
    delay(1000);
    long initial_homingy = 6000;  // Used to Home Stepper at startup. Must be away from home
    stepperY.setCurrentPosition(initial_homingy);
    while (analogRead(ruptPiny) > 600 && sensorFaily == false) {
      stepperY.moveTo(initial_homingy);  // Set the position to move to
      initial_homingy--;                 // Decrease by 1 for next move if needed
      stepperY.run();                    // Start moving the stepper
    }
    int backStep = 100;
    stepperY.setCurrentPosition(0);
    stepperY.moveTo(-10);
    stepperY.runToPosition();
    stepperY.setSpeed(1);
    stepperY.setAcceleration(1);
    while (analogRead(ruptPiny) < 600 && sensorFaily == false) {
      stepperY.moveTo(backStep);  // Move into flag 10 steps
      backStep++;                 // Increase by 1 for next move if needed
      stepperY.runSpeed();        // Start moving the stepper
    }
    Serial.println("Y homed");
    Serial.print(yPosition);
    Serial.println(F(", "));
    stepperY.setCurrentPosition(0);
    yPosition = stepperY.currentPosition();
    stepperY.setSpeed(800);
    stepperY.setAcceleration(200);
    sensorFaily = false;
  }
}
void setPosition() {
  if (axis == "x") {
    stepperX.setCurrentPosition(requestedPulses);
    Serial.print(F("Current X = "));
    Serial.println(requestedPulses);
  } else if (axis == "y") {
    stepperY.setCurrentPosition(requestedPulses);
    Serial.print(F("Current Y = "));
    Serial.println(requestedPulses);
  }
}

void testMotor() {
  stepperX.move(numSteps);   // Set desired move: 800 steps (in 1/8 resolution that's one rotation)
  stepperX.runToPosition();  // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position
  delay(1000);
  stepperX.move(-1 * numSteps);  // Set desired move: 800 steps (in 1/8 resolution that's one rotation)
  stepperX.runToPosition();      // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position
  delay(1000);

  stepperY.move(numSteps);   // Set desired move: 800 steps (in 1/8 resolution that's one rotation)
  stepperY.runToPosition();  // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position
  delay(1000);
  stepperY.move(-1 * numSteps);  // Set desired move: 800 steps (in 1/8 resolution that's one rotation)
  stepperY.runToPosition();      // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position
  delay(1000);
}