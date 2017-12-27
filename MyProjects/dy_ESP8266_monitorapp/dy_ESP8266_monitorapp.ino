/*
* ESP8266 WiFi - Monitoring sensors
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
const char* SRV_URL    = "192.168.1.24";
const int SRV_PORT     = 2201;
const char* USER_ID    = "1001";
const char* DEVICE_ID  = "123456";
const int DELAY_UPLOAD = 60000;
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const int sensorNum = 11;

//STRUCT  {"sensor_address":"3563547","sensor_type":"temp","sensor_value":"22.22"}
//------------------------------------------------------------------------------------------------------------------
typedef struct{
    String sensor_id;
    DeviceAddress sensor_address;
    String sensor_value;
} TSensor;
TSensor sensorsArr[sensorNum];


//VARs

//OTHER
#define serial Serial
#define PIN_ONE_WIRE_BUS D4  // D4 -> gpio 2 
OneWire oneWire(PIN_ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

#define DHTTYPE DHT11       // DHT 11
#define PIN_DHT D3          // D1 -> gpio 5
DHT dht(PIN_DHT, DHTTYPE);  // Initialize DHT sensor.

WiFiClient client;

//******************************************************************************************************************************
void setup() {
  serial.begin(115200);
  while (!serial) {;}   //Initialize serial and wait for port to open:
  serial.println("");
  serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  serial.println("ESP8266 WiFi - Monitoring sensors");
  delay(100);

  defineDeviceSensors();

  // Initialize measurement library
  DS18B20.begin();    
  dht.begin();

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
  serial.print("device IP address: "); 
  serial.println(WiFi.localIP()); 
  serial.println("----------------------------------------------------------------------------------");


  //  test
  //----------------------------------------------------------

}

//******************************************************************************************************************************
// SENSOR MAPPING
/*
  SID (Sensor ID) -> ADDRESS
  -------------------------------
  100 -> 28 06 fe 5b 05 00 00 d1  - S6  - puffer 4
  101 -> 28 29 a0 5b 05 00 00 74  - S5  - radi topla
  102 -> 28 ff 88 f1 30 17 04 7e  - S7  - puffer 3
  103 -> 28 ff 8a 22 31 17 04 df  - S4  - radi hladna
  104 -> 28 ff 4e 6c 31 17 03 94  - S1  - ckp core
  105 -> 28 ff 95 1a 31 17 04 98  - S2  - ckp pol
  106 -> 28 ff c7 0c 31 17 03 3c  - S8  - puffer 2  
  107 -> 28 ff b7 b8 30 17 04 e6  - S3  - ckp pov
  108 -> 28 ff 2f 15 31 17 03 73  - S9  - puffer 1
  109 -> 28 ff b8 1c 31 17 04 85  - S10 - soba
  110 -> 28 ff 67 b6 30 17 04 ad  - S11 - vani

  109 -> 28 ff 67 b6 30 17 04 ad  - nema
  107 -> 28 ff 75 1f 31 17 03 40  - nema
  200 -> 10 e3 c4 71 01 08 00 4c - S10 - soba
*/

void defineDeviceSensors(){
    //DeviceAddress -> typedef uint8_t DeviceAddress[8];
    // add values to sensor:    sensor_id  |  sensor_address  |  sensor_value;
    sensorsArr[0]   = (TSensor) {"100", {0x28, 0x06, 0xfe, 0x5b, 0x05, 0x00, 0x00, 0xd1}, ""};
    sensorsArr[1]   = (TSensor) {"101", {0x28, 0x29, 0xa0, 0x5b, 0x05, 0x00, 0x00, 0x74}, ""};
    sensorsArr[2]   = (TSensor) {"102", {0x28, 0xff, 0x88, 0xf1, 0x30, 0x17, 0x04, 0x7e}, ""};  
    sensorsArr[3]   = (TSensor) {"103", {0x28, 0xff, 0x8a, 0x22, 0x31, 0x17, 0x04, 0xdf}, ""};
    sensorsArr[4]   = (TSensor) {"104", {0x28, 0xff, 0x4e, 0x6c, 0x31, 0x17, 0x03, 0x94}, ""};
    sensorsArr[5]   = (TSensor) {"105", {0x28, 0xff, 0x95, 0x1a, 0x31, 0x17, 0x04, 0x98}, ""};
    sensorsArr[6]   = (TSensor) {"106", {0x28, 0xff, 0xc7, 0x0c, 0x31, 0x17, 0x03, 0x3c}, ""};    
    sensorsArr[7]   = (TSensor) {"107", {0x28, 0xff, 0xb7, 0xb8, 0x30, 0x17, 0x04, 0xe6}, ""};
    sensorsArr[8]   = (TSensor) {"108", {0x28, 0xff, 0x2f, 0x15, 0x31, 0x17, 0x03, 0x73}, ""};
    sensorsArr[9]   = (TSensor) {"109", {0x28, 0xff, 0xb8, 0x1c, 0x31, 0x17, 0x04, 0x85}, ""};
    sensorsArr[10]  = (TSensor) {"110", {0x28, 0xff, 0x67, 0xb6, 0x30, 0x17, 0x04, 0xad}, ""};
    
}

//sensorsArr[9]  = (TSensor) {"109", {0x10, 0xe3, 0xc4, 0x71, 0x01, 0x08, 0x00, 0x4c}, ""}; //stari senzor
//sensorsArr[7]  = (TSensor) {"108", {0x28, 0xff, 0x75, 0x1f, 0x31, 0x17, 0x03, 0x40}, ""};
//sensorsArr[9]  = (TSensor) {"110", {0x28, 0xff, 0x67, 0xb6, 0x30, 0x17, 0x04, 0xad}, ""};
//sensorsArr[3]  = (TSensor) {"104", {0x28, 0xff, 0xb8, 0x1c, 0x31, 0x17, 0x04, 0x85}, ""};

//******************************************************************************************************************************
String getStringFromDeviceAddress(DeviceAddress devAdr){
  //String result = String(devAdr[5], HEX) + "-" + String(devAdr[6], HEX) + "-" + String(devAdr[7], HEX);
  //String result = getFormatedStrFromHex8(&devAdr[5],1) + "-" + getFormatedStrFromHex8(&devAdr[6],1) + "-" + getFormatedStrFromHex8(&devAdr[7],1);
  String result = getFormatedStrFromHex8(devAdr, 8);
  return result;  
}


float readSensorDHT11Temp(){       
   float tempVal = dht.readTemperature();
   if (isnan(tempVal)) {
      serial.println("readSensorDHT11Temp - FAILED!");       
   }else{
      serial.println("readSensorDHT11Temp " + String(tempVal));
   }
   return tempVal;
}

float readSensorDHT11Humidity(){       
   float humVal = dht.readHumidity();
      if (isnan(humVal)) {
      serial.println("readSensorDHT11Humidity - FAILED!");       
   }else{
      serial.println("readSensorDHT11Humidity " + String(humVal));
   }
   return humVal;
}

void calculateDHT11Vals(float rawTemp, float rawHum){
  // Computes temperature values in Celsius and Humidity
  float hic = dht.computeHeatIndex(rawTemp, rawHum, false);       
  serial.println("calculateDHT11Vals -> " + String(hic));
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
  
  /* DHT11
  //-------------------------------------------------------------------------
  float sensVal2 = readSensorDHT11Temp();
  if (!isnan(sensVal2)){
    sensorsArr[2].sensor_value = String(sensVal2);
  }*/
  
}

// Open connection to the HTTP server
bool connect(const char* hostName, unsigned port) {
  serial.println("Connect to " + String(hostName));
  bool ok = client.connect(hostName, port);
  serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* host, const char* route) {
  serial.println("POST " + String(route));
  //String postData = "{\"user_id\" : \"1001\", \"device_id\" : \"123456\", \"sensor_id\" : \"3563547\", \"sensor_value\" : \"6666\"}";
  //serial.println("postData " + String(postData));

  String postData = prepareJOSNStringFromSensorsArr();
  serial.println("postData " + String(postData));

  client.print("POST ");
  client.print(route);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(host);
  client.println("Cache-Control: no-cache");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(postData.length());
  client.println();
  client.println(postData);

  return true;
}

String prepareJOSNStringFromSensorsArr(){
  String jsonStr = "{ \"user_id\" : \"" + String(USER_ID) + "\", \"device_id\" : \"" + String(DEVICE_ID) + "\", "; 
  jsonStr += "\"sensors\": [";
  
  serial.println("numOfSensors -> " + String(sensorNum));
  String sens = "";
  for (int i=0; i < sensorNum; i++){  
    sens = sens + "{\"sensor_id\":\"" + String(sensorsArr[i].sensor_id) + "\",\"sensor_value\":\"" + String(sensorsArr[i].sensor_value) + "\"}";
    if (i < (sensorNum-1)){
      sens += ",";
    }
  }
  jsonStr += sens;
  jsonStr += "]";   //end of sens array
  jsonStr += "}";   //end of json object

  return jsonStr;
}


// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    Serial.println("No response or invalid response!");
  }

  return ok;
}

void readReponseContent() {
    // if there are incoming bytes available from the server, read them and print them:
  //while (client.available()) {
   // char response = client.read();
   // serial.println("server response: " + response);
  //}
}

// Close the connection with the HTTP server
void disconnect() {
  Serial.println("Disconnect");
  client.stop();
}

// Pause for a 1 minute
void wait() {
  Serial.println("Wait 1 minute");
  delay(DELAY_UPLOAD);
}

//******************************************************************************************************************************
void loop() {

  readSensors();  //after readings is complete value is stored in sensorsArr[TSensor]
  
  if (connect(SRV_URL, SRV_PORT)) {
    if (sendRequest(SRV_URL, "/storedevicedata") && skipResponseHeaders()) {
      readReponseContent();
    }
  }
  disconnect();
  wait();
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
