/*
 * Module for Mobile & RC Receiver 6CH communication
 *   Created on: 28.04.2017
 *   by DubyOriginal
 *  
 * ---------------------------------------------------------  
 * -> Arduino MEGA ADK
 * -> FlySky Receiver 6CH
 * 
 * --------------------------------------------------------- 
 * source: USB mini 3.3V
 * 
 * ---------------------------------------------------------
 * Programmer setup:
 *    - Tools -> Board -> Generic ESP8266 Module
 *    - CPU 80MHz, Flash 40MHz
 *    - Flash size: 4MB
 */

#include <Arduino.h>


// PINS (defined in BasicUtils)
//---------------------------------------------------------------  
const int analogPin = 0;
const int B_LED = 13;
const int pinCH1 = 22;
const int pinCH2 = 24;
const int pinCH3 = 26;  
const int pinCH4 = 28;
const int pinCH5 = 30;  

// CONST
//---------------------------------------------------------------

// VAR
//---------------------------------------------------------------


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
    serial.println("Module for Mobile & RC Receiver 6CH communication");
    delay(100);
    
}


//*********************************************************************************************************
void preparePINS(){
    pinMode(B_LED, OUTPUT); 
    pinMode(analogPin, INPUT);   // Sets the analogPin as an Input
    pinMode(pinCH1, INPUT);   
    pinMode(pinCH2, INPUT);
    pinMode(pinCH3, INPUT);
    pinMode(pinCH4, INPUT);
    pinMode(pinCH5, INPUT);
}

void initialBlink(){
    digitalWrite(B_LED, 1);
    delay(700);
    digitalWrite(B_LED, 0);
    delay(700);
    digitalWrite(B_LED, 1);
    delay(700);
    digitalWrite(B_LED, 0);
    delay(700);
    digitalWrite(B_LED, 1);
    delay(700);
    digitalWrite(B_LED, 0);
}


//**************************************************************************************************************************************
//**************************************************************************************************************************************

//**************************************************************************************************************************************
//**************************************************************************************************************************************
void listenReceiverEvent(){
  //toggle led
  digitalWrite(B_LED, !digitalRead(B_LED));

  int ch1;
  int ch2;
  int ch3;
  int ch4;
  int ch5;
  int CHANNEL[6];

  int ch1RAW = 0;
  int ch2RAW = 0;
  int ch3RAW = 0;
  int ch4RAW = 0;
  int ch5RAW = 0;

  // Read the pulse width of each channel
  ch1RAW = pulseIn(pinCH1, HIGH, 30000); 
  ch2RAW = pulseIn(pinCH2, HIGH, 30000);  
  ch3RAW = pulseIn(pinCH3, HIGH, 30000);
  ch4RAW = pulseIn(pinCH4, HIGH, 30000);
  ch5RAW = pulseIn(pinCH5, HIGH, 30000);

  //Serial.println("RAW CH 1:" + String(ch1RAW) + ",\t CH 2:" + String(ch2RAW) + ",\tCH 3:" + String(ch3RAW) + ",\tCH 4:" + String(ch4RAW) + ",\tCH 5:" + String(ch5RAW));

  //move = map(ch2, 1000,2000, -500, 500); //center over zero
  ch1 = map(ch1RAW, 985, 1995, 0, 1023);
  ch2 = map(ch2RAW, 985, 1995, 0, 1023);
  ch3 = map(ch3RAW, 985, 1995, 0, 1023);
  ch4 = map(ch4RAW, 985, 1995, 0, 1023);
  ch5 = map(ch5RAW, 985, 1995, 0, 1023);

  int offset = 0;
  CHANNEL[1] = ch1 - offset;
  CHANNEL[2] = ch2 - offset;
  CHANNEL[3] = ch3 - offset;
  CHANNEL[4] = ch4 - offset;
  CHANNEL[5] = ch5 - offset;

  String chStr1 = "NC";
  String chStr2 = "NC";
  String chStr3 = "NC";
  String chStr4 = "NC";
  String chStr5 = "NC";
  
  if (CHANNEL[1] > 0){chStr1 = String(CHANNEL[1]);} 
  if (CHANNEL[2] > 0){chStr2 = String(CHANNEL[2]);}
  if (CHANNEL[3] > 0){chStr3 = String(CHANNEL[3]);}
  if (CHANNEL[4] > 0){chStr4 = String(CHANNEL[4]);}
  if (CHANNEL[5] > 0){chStr5 = String(CHANNEL[5]);}

  Serial.println("CH 1:" + chStr1 + ",\t CH 2:" + chStr2 + ",\tCH 3:" + chStr3 + ",\tCH 4:" + chStr4 + ",\tCH 5:" + chStr5);

  //encodeChannelValues(CHANNEL);

  delay(50); 
}
//**************************************************************************************************************************************
//**************************************************************************************************************************************

String forwardCMD(String params){
    serial.println("FORWARD CMD: " + params);
    return "OK";
}

//encode receiver readings to CMD for drive controll
void encodeChannelValues(int channel[]){
  Serial.println("ENC -> CH 1:" + String(channel[1]) + ",\t CH 2:" + String(channel[2]) + ",\tCH 3:" + String(channel[3]) + ",\tCH 4:" + String(channel[4]) + ",\tCH 5:" + String(channel[5]));
}

//**************************************************************************************************************************************
//**************************************************************************************************************************************
void loop() {

    listenReceiverEvent();
}



