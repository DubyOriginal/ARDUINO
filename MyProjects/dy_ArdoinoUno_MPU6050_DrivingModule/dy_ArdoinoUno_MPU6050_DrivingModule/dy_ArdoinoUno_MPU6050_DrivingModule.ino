/*
* Arduino Uno + Driving module + MPU6050 
*   Created on: 19.04.2017
*   by DubyOriginal
*  
* ---------------------------------------------------------  
* -> Arduino Uno
* -> MPU6050 - gyroscope and accelerometer
* -> L298N - motor driver
* 
* --------------------------------------------------------- 
* source: Battery
* -> L298N 5V -> ArdUno Vin pin
* 
* ---------------------------------------------------------
* Programmer setup:
*    - Tools -> Board -> Arduino Uno
*/

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
// download url: https://github.com/AritroMukherjee/MPU5060sensor

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  #include "Wire.h"
#endif


// CONST
//---------------------------------------------------------------
#define INTERRUPT_PIN 2  // use pin 2 on Arduino Uno & most boards
//#define LED LED_BUILTIN // (Arduino is 13)

// motorA
const int enA = 8;   //zelena
const int in1 = 9;   //crvena
const int in2 = 10;   //ljub.
// motorB
const int in3 = 11;   //smeđa
const int in4 = 12;   //plava
const int enB = 13;   //žuta

//Drive
const int S_FORWARD = 10;
const int S_BACK    = 20;
const int S_RLEFT   = 30;
const int S_RRIGHT  = 40;
const int S_STOP    = 50;

//VARS
//---------------------------------------------------------------
bool blinkState = true;

// MPU control/status vars
//----------------------------
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };

// Motor driver VARs
//----------------------------
long duration;
int distance;
unsigned long t0 = 0;
int driveState = S_STOP;   //initial value
int dSpeed = 120;        //initial value

//OTHER
//---------------------------------------------------------------
MPU6050 mpu;
#define serial Serial

//***************************************************************************************************************************
// INTERRUPT DETECTION ROUTINE 
//***************************************************************************************************************************
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}

//***************************************************************************************************************************
void setup() {
  serial.begin(115200);  
  serial.println("");
  serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  serial.println("Motor Driver v1");
  serial.println("starting...");

  preparePINS();
  driveStop();  
  //initialBlink();

  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
      Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif

  // initialize device
  serial.println(F("Initializing I2C devices..."));
  mpu.initialize();

  // verify connection
  serial.println(F("Testing device connections..."));
  serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // wait for ready
  serial.println(F("\nSend any character to begin DMP programming and demo: "));

  // load and configure the DMP
  serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
      // turn on the DMP, now that it's ready
      serial.println(F("Enabling DMP..."));
      mpu.setDMPEnabled(true);

      // enable Arduino interrupt detection
      serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
      attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
      mpuIntStatus = mpu.getIntStatus();

      // set our DMP Ready flag so the main loop() function knows it's okay to use it
      serial.println(F("DMP ready! Waiting for first interrupt..."));
      dmpReady = true;

      // get expected DMP packet size for later comparison
      packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
      // ERROR!
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      // (if it's going to break, usually the code will be 1)
      serial.print(F("DMP Initialization failed (code "));
      serial.print(devStatus);
      serial.println(F(")"));
  }
}

//*********************************************************************************************************
void preparePINS(){
    //pinMode(LED, OUTPUT);
    pinMode(INTERRUPT_PIN, INPUT);
  
    //drive module
    //----------------------------
    // motorA
    pinMode(enA, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    // motorB
    pinMode(enB, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
}

/*
void initialBlink(){
    digitalWrite(LED, 1);
    delay(700);
    digitalWrite(LED, 0);
    delay(700);
    digitalWrite(LED, 1);
    delay(700);
    digitalWrite(LED, 0);
    delay(700);
    digitalWrite(LED, 1);
    delay(700);
    digitalWrite(LED, 0);
}*/

//*********************************************************************************************************
String handleEventForCode(String msgCode){
  serial.println("handleWSEventForCode - " + msgCode);
  String resultStr = "";
  if (msgCode=="CMD_TEST"){
    serial.println("TEST event");
    resultStr = "RES_TEST,OK";
  //---------------------------------------------------
  }else if (msgCode.startsWith("D_MANUAL")){
    serial.println("execute D_MANUAL event");
    String responseValue = prepareAndRunManualDrive(msgCode);
    resultStr = "RES_MANUAL," + responseValue;

  }else if (msgCode == "D_FORWARD"){
    serial.println("execute D_FORWARD event");
    driveFroward();
    resultStr = "RES_FORWARD,OK";

  }else if (msgCode == "D_BACK"){
    serial.println("execute D_BACK event");
    driveBack();
    resultStr = "RES_BACK,OK";

  }else if (msgCode == "D_ROTLEFT"){
    serial.println("execute D_ROTLEFT event");
    rotateLeft();
    resultStr = "RES_ROTLEFT,OK";

  }else if (msgCode == "D_ROTRIGHT"){
    serial.println("execute D_ROTRIGHT event");
    rotateRight();
    resultStr = "RES_ROTRIGHT,OK";

  }else if (msgCode == "D_STOP"){
    serial.println("execute D_STOP event");
    driveStop();
    resultStr = "RES_STOP,OK";
    
  }else if (msgCode == "D_SPEED"){
    serial.println("execute D_SPEED event");
    updateSpeed(msgCode);
    resultStr = "RES_SPEED,OK";
  }else {
    serial.println("unknown message code");
  }
  return resultStr;
}

//**************************************************************************************************************************************
//**************************************************************************************************************************************

//******************************************************************************
String prepareAndRunManualDrive(String params){
    //params: "D_MANUAL,-125,320"
    int comma1Index = params.indexOf(',');
    int comma2Index = params.indexOf(',', comma1Index + 1);  //  Search for the next comma just after the first

    String firstValue = params.substring(0, comma1Index);
    String motAStr = params.substring(comma1Index + 1, comma2Index);
    String motBStr = params.substring(comma2Index + 1); // To the end of the string

    serial.println(String("motorA: " +  motAStr + ", motorB: " + motBStr));
    int motA = motAStr.toInt();
    int motB = motBStr.toInt();
    manualDrive(motA, motB);
    return "OK";
}

//MANUAL DRIVE CONTROL
//*********************************************************************************************************
void manualDrive(int mSpeedA, int mSpeedB){
  //MOTOR A
  //*********************************************
  if (mSpeedA < 0){
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    mSpeedA = -1 * mSpeedA;
    
  }else if (mSpeedA > 0){
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  
  }else{
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
  analogWrite(enA, mSpeedA);

  //MOTOR B
  //*********************************************
  if (mSpeedB < 0){
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    mSpeedB = -1 * mSpeedB;
    
  }else if (mSpeedB > 0){
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  
  }else{
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
  }
  analogWrite(enB, mSpeedB);

}

//SMART DRIVE CONTROL
//*********************************************************************************************************
void driveFroward(){
  serial.print("drive: FORWARD (");
  serial.print(dSpeed);
  serial.println(")");
  driveState = S_FORWARD;
 
  // turn on motor A
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, dSpeed);
}

//******************************************************************************
void driveBack(){
  serial.print("drive: BACK (");
  serial.print(dSpeed);
  serial.println(")");
  driveState = S_BACK;
  
  // turn on motor A
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, dSpeed);
}

//******************************************************************************
void rotateLeft(){
  serial.print("drive: RLEFT (");
  serial.print(dSpeed);
  serial.println(")");
  driveState = S_RLEFT;
  
  // turn on motor A
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, dSpeed);
}

//******************************************************************************
void rotateRight(){
  serial.print("drive: RRIGHT (");
  serial.print(dSpeed);
  serial.println(")");
  driveState = S_RRIGHT;
  
  // turn on motor A
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, dSpeed);
}

//******************************************************************************
void driveStop(){
  serial.println("drive: STOP");
  driveState = S_STOP;
  
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  analogWrite(enA, 0);
  analogWrite(enB, 0);
}

//******************************************************************************
void updateSpeed(String params){
    
    //params: "D_SPEED,620"
    int paramsLenght = params.length();
    int comma1Index = params.indexOf(',');
    String firstValue = params.substring(0, comma1Index);    //D_SPEED
    String dSpeedStr = params.substring(comma1Index + 1, paramsLenght);   //620
    dSpeed = dSpeedStr.toInt();
    serial.println("updateSpeed: " +  dSpeedStr);
    
    if (driveState == S_FORWARD){
        driveFroward();
    } else if (driveState == S_BACK){
        driveBack();
    } else if (driveState == S_RLEFT){
        rotateLeft();
    } else if (driveState == S_RRIGHT){
        rotateRight();
    }
}

void readMPU6050Sensor(){
  serial.print("R");
    // if programming failed, don't try to do anything
  if (!dmpReady) return;

  // wait for MPU interrupt or extra packet(s) available
  while (!mpuInterrupt && fifoCount < packetSize) {
    serial.print("*");
  }

  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
      // reset so we can continue cleanly
      mpu.resetFIFO();
      serial.println(F("FIFO overflow!"));

  // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
      // wait for correct available data length, should be a VERY short wait
      while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

      // read a packet from FIFO
      mpu.getFIFOBytes(fifoBuffer, packetSize);
      
      // track FIFO count here in case there is > 1 packet available
      // (this lets us immediately read more without waiting for an interrupt)
      fifoCount -= packetSize;

      // display Euler angles in degrees
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetEuler(euler, &q);
      int angX = euler[0] * 180/M_PI;
      int angY = euler[1] * 180/M_PI;
      int angZ = euler[2] * 180/M_PI;
      serial.println("X: " + String(angX) + " | Y: " + String(angY) + " | Z: " + String(angZ));

      if (angX > 0 && angX < 180){
        driveState = S_FORWARD;
        int mapS = map(angX, 0, 180, 0, 1024);
        String uSpeedCMD = "D_SPEED," + String(mapS);
        updateSpeed(uSpeedCMD);
      }else {
        driveStop();  
      }

      // blink LED to indicate activity
      //blinkState = !blinkState;
      //digitalWrite(LED, blinkState);
  }
}


//***************************************************************************************************************************
void loop() {
  readMPU6050Sensor();
  delay(50);
}
