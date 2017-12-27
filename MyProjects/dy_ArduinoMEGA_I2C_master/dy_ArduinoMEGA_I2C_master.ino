/*
* Arduino MEGA ADK as I2C master - communication between arduinos
 *   Created on: 09.07.2017
 *   by DubyOriginal
 *  
 * ---------------------------------------------------------  
 * -> Arduino MEGA ADK
 * 
 * --------------------------------------------------------- 
 * source: USB 5V
 * 
 * ---------------------------------------------------------
 * Programmer setup:
 *    - Tools -> Board -> Arduino MEGA ADK
 */

#include <Arduino.h>
#include <Wire.h>    //declare I2C library function


// PINS (defined in BasicUtils)
//---------------------------------------------------------------  
const int analogPin = 0;
const int B_LED = 13;


// CONST
//---------------------------------------------------------------
const byte MASTER_ID = 99;
const byte SLAVE_ID = 11;

// VAR
//---------------------------------------------------------------
char inBuffer[10]={};    //empty array where to put the numbers comming from the slave

//OTHER
//---------------------------------------------------------------
#define serial Serial

//**************************************************************************************************************************************
//**************************************************************************************************************************************
void setup() {
    serial.begin(115200);

    preparePINS();
    initialBlink();

    serial.println("");
    serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    serial.println("Arduino MEGA ADK as I2C master");
    delay(100);

    Wire.begin();        // join i2c bus (address optional for master) 
}

//*********************************************************************************************************
void preparePINS(){
    pinMode(B_LED, OUTPUT); 
}

void initialBlink(){
    //digitalWrite(B_LED, 1);
    //delay(700);
}

//**************************************************************************************************************************************
void loop() {
  byte numOfByte = 10;
  Wire.requestFrom(SLAVE_ID, numOfByte);    // request 3 bytes from slave device 

  //gathers data comming from slave
  int i=0; //counter for each bite as it arrives
  while (Wire.available()) { 
    inBuffer[i] = Wire.read(); // every character that arrives it put in order in the empty array inBuffer
    i=i+1;
  }

  //String output = "data from slave: " + String(t[0]) + ", " + String(t[1]) + ", " + String(t[2]);
  String output = "data from slave: " + String(inBuffer);
  Serial.println(output);   //shows the data in the array t
  delay(500); //give some time to relax

  // send data to slave. here I am just sending the number 44
  Wire.beginTransmission(SLAVE_ID);
  Wire.write(55);
  Wire.endTransmission();
}



