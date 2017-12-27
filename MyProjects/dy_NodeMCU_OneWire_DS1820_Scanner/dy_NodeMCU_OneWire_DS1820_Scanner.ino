/*
* NodeMCU LoLin - DS18B20 temp sensor (ONE WIRE) 
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

#include <OneWire.h>

// CONST
//---------------------------------------------------------------
#define LED LED_BUILTIN // (Arduino is 13)
#define D4 2 // DS18B20 data pin


//VARS
//---------------------------------------------------------------
float prevTemp = 0;
byte devCnt = 0;

//OTHER
//---------------------------------------------------------------
#define serial Serial
OneWire oneWire(D4);  // Create a 1-wire object

//***************************************************************************************************************************
void setup() {
  serial.begin(115200);  
  serial.println("");
  serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  serial.println("NodeMCU LoLin - DS18B20 temp sensor");
  serial.println("starting...");

  // setup OneWire bus
  //DS18B20.begin();

}

// FUNCTIONS
//*********************************************************************************************************

//****************************************************************************************
void scanOneWireDevices(){
  byte addr[8];

  serial.println("--------------------------------------------------");
  serial.println("Start scanning for devices....");
  
  while(oneWire.search(addr)) {
    devCnt++;
    serial.println("");

    //String devAddrStr = String(addr[0]) + "-" + String(addr[1]) + "-" + String(addr[2]) + "-" + String(addr[3]) + "-" + String(addr[4]) + "-" + String(addr[5]) + "-" + String(addr[6]) + "-" + String(addr[7]);
    serial.println("  SENSOR[" + String(devCnt) + "]");
 
    getDataForDevice(addr);
  }

  devCnt = 0;
  oneWire.reset_search();
  serial.println();
  serial.println("No more devices!");
  
  delay(250);

}

void getDataForDevice(byte devAddr[8]){
  byte type_s;

    // the first ROM byte indicates which chip
  String chipStr = "";
  switch (devAddr[0]) {
    case 0x10:
      chipStr = "DS18S20";  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      chipStr = "DS18B20";
      type_s = 0;
      break;
    case 0x22:
      chipStr = "DS1822";
      type_s = 0;
      break;
    default:
      chipStr = "Not a DS18x20 family device.";
      return;
  } 
  
  
  String ROMStr = String(devAddr[0], HEX);
  for(byte i = 1; i < 8; i++) {
    ROMStr = ROMStr + " " + String(devAddr[i], HEX);
  }

  serial.print("  CHIP: " + chipStr + " -> ROM: " + ROMStr);

  if (OneWire::crc8(devAddr, 7) != devAddr[7]) {
      serial.println("CRC is not valid!");
      return;
  }

  serial.println();

  byte data[12];
  readRawTempFromSensor(devAddr, data); //NOTE - return data is stored in param data!
  float celsius = getTempFromRawData(data, type_s);
  serial.println("  Temp = " + String(celsius) + "'C");
}

void readRawTempFromSensor(byte devAddr[8], byte data[12]){;
  oneWire.reset();
  oneWire.select(devAddr);
  oneWire.write(0x44);        // start conversion, use ds.write(0x44,1) with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  byte present = 0;
  present = oneWire.reset();
  oneWire.select(devAddr);    
  oneWire.write(0xBE);         // Read Scratchpad
  
  for (byte i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = oneWire.read();
  } 
}

// Convert the data to actual temperature because the result is a 16 bit signed integer, it should
// be stored to an "int16_t" type, which is always 16 bits even when compiled on a 32 bit processor.
float getTempFromRawData(byte data[12], byte type_s){
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  return (float)raw / 16.0;  //for celsius
}

//***************************************************************************************************************************
void loop() {
  //float temperature = getTemperature();
  //serial.println("Temp: " + String(temperature));
  
  //discoverOneWireDevices(); 
  scanOneWireDevices();
  
  delay(5000);
}








