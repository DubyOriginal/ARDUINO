/*
* ESP8266 WiFi - Basic sensor connection
 *   Created on: 17.09.2017
 *   by DubyOriginal
 *  
 * ---------------------------------------------------------  
 * -> Arduino ESP8266
 * 
 * --------------------------------------------------------- 
 * source: USB 5V
 *
  PINOUT (DHT11 left to right):
    1. Vcc
    2. Data (R > 1k  connect to VCC)
    3. not used
    4. GND
 * ---------------------------------------------------------
 * Programmer setup:
 *    - Tools -> Board -> NodeMCU 1.0
 */

 
#include <ESP8266WiFi.h> 
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "BasicUtils.h"

//CONST
const char* ssid       = "SkyNet";
const char* password   = "adidasneo";
const char* SRV_URL    = "192.168.1.27";
const int REFRESH_TIMEOUT = 5000;            //refresh time
const int sensorNum = 2;                  //num of sensors

//STRUCT  {"sensor_address":"3563547","sensor_type":"temp","sensor_value":"22.22"}
//------------------------------------------------------------------------------------------------------------------
typedef struct{
    String sensor_id;
    DeviceAddress sensor_address;
    String sensor_value;
} TSensor;
TSensor sensorsArr[sensorNum];

//OTHER
#define serial Serial
#define D4 2  // D4 -> gpio 2 
OneWire oneWire(D4);
DallasTemperature DS18B20(&oneWire);

WiFiClient client;

//******************************************************************************************************************************
void setup() {
  serial.begin(115200);
  while (!serial) {;}   //Initialize serial and wait for port to open:
  serial.println("");
  serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  serial.println("ESP8266 WiFi - Basic sensors");
  delay(100);

  defineDeviceSensors();

  // Initialize measurement library
  DS18B20.begin();    

  //init WiFi 
  //----------------------------------------------------------
  WiFi.begin (ssid, password);
  WiFi.mode(WIFI_STA);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ); 
    serial.print ( "." );
  }
  serial.println ("");
  serial.println ("Connected to " + String(ssid));
  serial.println("device IP address: " + String(WiFi.localIP())); 


  //  test
  //----------------------------------------------------------

}

//******************************************************************************************************************************
  //CHIP: DS18B20 -> ROM: 28 ff 0c 00 a3 16 05 02
  //CHIP: DS18B20 -> ROM: 28 ff 69 0c a3 16 03 0e
void defineDeviceSensors(){
    //DeviceAddress -> typedef uint8_t DeviceAddress[8];
    // add values to sensor:    sensor_id  |  sensor_address  |  sensor_value;
    sensorsArr[0]   = (TSensor) {"100", {0x28, 0xff, 0x0c, 0x00, 0xa3, 0x16, 0x05, 0x02}, ""};
    sensorsArr[1]   = (TSensor) {"101", {0x28, 0xff, 0x69, 0x0c, 0xa3, 0x16, 0x03, 0x0e}, ""};  
}

//******************************************************************************************************************************
String getStringFromDeviceAddress(DeviceAddress devAdr){
  String result = getFormatedStrFromHex8(devAdr, 8);
  return result;  
}

float readSensorDS18ByIndex(int index){    //DeviceAddress deviceAddress){     
   float tempVal = DS18B20.getTempCByIndex(index);
   Serial.println("readSensorDS18ByIndex[" + String(index) + "] " + String(tempVal));
   return tempVal;
}


float readSensorDS18ByDeviceAddress(DeviceAddress deviceAddress){   
   float tempC = DS18B20.getTempC(deviceAddress);
   if (tempC <= -127.00) {
      serial.println("Error getting temperature  ");
   } else{
      serial.println("readSensorDS18ByDeviceAddress: " + String(tempC));
   }
   return tempC;
}


void readSensors(){
  serial.println("readSensors");
  DS18B20.requestTemperatures();

  // DS1820
  //-------------------------------------------------------------------------
  float sensVal = 0;
  for (int i=0; i < sensorNum; i++){
    sensVal = readSensorDS18ByDeviceAddress(sensorsArr[i].sensor_address);
    if (!isnan(sensVal)){
      sensorsArr[i].sensor_value = String(sensVal);
    }
  }
}


//******************************************************************************************************************************
void loop() {
  readSensors();  //after readings is complete value is stored in sensorsArr[TSensor]
    
  Serial.println("Wait for REFRESH_TIMEOUT");
  delay(REFRESH_TIMEOUT);
}



/*
{
  "user_id" : "1001", 
  "device_id" : "123456", 
  "sensors" : 
  [
    {"sensor_id":"3563547","sensor_type":"temp","sensor_value":"22.22"},
    {"sensor_id":"3563547","sensor_type":"hum","sensor_value":"60"}
  ] 
}
}*/
