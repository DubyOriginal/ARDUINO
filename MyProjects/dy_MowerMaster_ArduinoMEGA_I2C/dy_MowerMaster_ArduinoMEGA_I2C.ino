/*
 * Mower Master Controller
 *   - communication with MoverSlave (NodeMCu wifi & RC receiver)
 *   - handling drive event
 *   - handling sensor inputs
 *   
 *   Created on: 9.07.2017
 *   by DubyOriginal
 *  
 * ---------------------------------------------------------  
 * -> Arduino MEGA ADK
 * 
 * --------------------------------------------------------- 
 * source: USB mini 5V
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
const int pinCH1 = 22;
const int pinCH2 = 24;
 

// CONST
//---------------------------------------------------------------
const byte MY_ADDRESS = 25;
const byte SLAVE_ADDRESS = 42;

// VAR
//---------------------------------------------------------------
  int ch1 = -1;
  int ch2 = -1;
  int ch3 = -1;
  int ch4 = -1;
  int ch5 = -1;

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
    serial.println("Mower Master Controller");
    delay(100);

    initI2CProtocol();
    
}


//*********************************************************************************************************
void preparePINS(){
    pinMode(B_LED, OUTPUT); 

    pinMode(pinCH1, INPUT);   
    pinMode(pinCH2, INPUT);

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
// I2C - part
//*********************************************************************************************************
void initI2CProtocol(){
  Wire.begin(MY_ADDRESS);
  Wire.onReceive(receiveI2CEvent);

  serial.println("I2C initialized");
}


void receiveI2CEvent (int howMany){
  for (int i = 0; i < howMany; i++){
    byte b = Wire.read ();
    serial.print("CH" + String(i) + ": " + String(b));
    //digitalWrite (LED, b); 
  }  // end of for loop
} // end of receiveEvent


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
  // send data to slave. here I am just sending the number 2
  Wire.beginTransmission(8);
  Wire.write(55);
  Wire.endTransmission();
  
   Wire.requestFrom(SLAVE_ADDRESS, 5); // Request the transmitted two bytes from the two registers
   if(Wire.available()==2) {  // 
    //ch1 = Wire.read(); // Reads the data from the register
    //ch2 = Wire.read(); 
    //ch3 = Wire.read(); 
    //ch4 = Wire.read(); 
    //ch5 = Wire.read();   
    ch1 = Wire.read() << 8;          // X-MSB
    ch1 |= Wire.read();              // X-LSB
  }

  String chStr1 = String(ch1);
  String chStr2 = String(ch2);
  String chStr3 = String(ch3);
  String chStr4 = String(ch4);
  String chStr5 = String(ch5);
  
  Serial.println("CH 1:" + chStr1 + ",\t CH 2:" + chStr2 + ",\tCH 3:" + chStr3 + ",\tCH 4:" + chStr4 + ",\tCH 5:" + chStr5);


  delay(500);

}  // end of loop




