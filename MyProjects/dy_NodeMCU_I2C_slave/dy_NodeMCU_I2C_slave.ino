/*
* NodeMCU LoLin as I2C slave - communication between arduinos
*   Created on: 09.07.2017
*   by DubyOriginal
*  
* ---------------------------------------------------------  
* -> NodeMCU LoLin
* 
* --------------------------------------------------------- 
* source: Battery
* -> ArdUno Vin pin
* 
* ---------------------------------------------------------
* Programmer setup:
*    - Tools -> Board -> Arduino Uno
*    
* ---------------------------------------------------------
* PINOUT
#define D0 16
#define D1 5  // I2C Bus SCL (clock)
#define D2 4  // I2C Bus SDA (data)
#define D3 0  //
#define D4 2  // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO 
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3  // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)
*/

#include <Wire.h>

// CONST
//---------------------------------------------------------------
const byte MASTER_ID = 99;
const byte SLAVE_ID = 11;
#define LED LED_BUILTIN // (Arduino is 13)
#define D1 5 // I2C Bus SCL (clock)
#define D2 4 // I2C Bus SDA (data)


//VARS
//---------------------------------------------------------------
char t[10]; //empty array where to put the numbers going to the master
volatile int masterData; // variable used by the master to sent data to the slave

//OTHER
//---------------------------------------------------------------
#define serial Serial


//***************************************************************************************************************************
void setup() {
  serial.begin(115200);  
  serial.println("");
  serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  serial.println("ArduinoUNO as I2C slave - communication");
  serial.println("starting...");

  //pinMode(D2, INPUT);
  //pinMode(D1, INPUT_PULLUP);

  initI2CProtocol();

}


// I2C - part
//*********************************************************************************************************
void initI2CProtocol(){
  Wire.begin();   //4: SDA, 5: SCL 
  //Wire.begin(SLAVE_ID);               //join I2Cbus as slave with identified address
  Wire.onReceive(receiveI2CEvent);    //register a function to be called when slave receive a transmission from master
  Wire.onRequest(requestI2CEvent);    //register a function when master request data from this slave device

  serial.println("I2C initialized");
}

// function: what to do when asked for data
void requestI2CEvent() {
  serial.println("requestI2CEvent");
  Wire.write(t); 
}

// what to do when receiving data from master
void receiveI2CEvent(int howMany){
  serial.println("receiveI2CEvent -> cnt: " + String(howMany));
  masterData = Wire.read();
}

//***************************************************************************************************************************
void loop() {
  t[0] = 65;
  t[1] = 66;
  t[2] = 67;

  serial.println(masterData);         // print the character
  delay(50);
}








