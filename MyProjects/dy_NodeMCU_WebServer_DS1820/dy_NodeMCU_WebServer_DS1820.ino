/*
* ESP8266 WiFi - Basic WebServer with DS1820
 *   Created on: 17.09.2017
 *   by DubyOriginal
 *  
 * ---------------------------------------------------------  
 * -> Arduino ESP8266
 * 
 * --------------------------------------------------------- 
 * source: USB 5V
 *
 * ---------------------------------------------------------
 * Programmer setup:
 *    - Tools -> Board -> Generic ESP8266 module
 */
 
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//CONST
const char* ssid = "SkyNet";
const char* password = "adidasneo";
const int sensorNum = 2;
const int REFRESH_TIMEOUT = 5000;            //refresh time

//STRUCT  {"sensor_address":"3563547","sensor_type":"temp","sensor_value":"22.22"}
typedef struct{
    String sensor_id;
    DeviceAddress sensor_address;
    String sensor_value;
} TSensor;
TSensor sensorsArr[sensorNum];

// Data wire is plugged into pin D4 on the ESP8266 12-E - GPIO 2
#define ONE_WIRE_BUS 2
#define serial Serial

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature DS18B20(&oneWire);
//char temperatureCString[6];
//char temperatureFString[6];

// Web Server on port 80
WiFiServer server(80);

// only runs once on boot
void setup() {
  serial.begin(115200);
  while (!serial) {;}   //Initialize serial and wait for port to open:
  serial.println("");
  serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  serial.println("ESP8266 WiFi - WebServer with DS1820");
  delay(100);

  getAllAttachedSensors();

  defineDeviceSensors();
  DS18B20.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  
  initWiFi_Server();
  
}

//******************************************************************************************************************************
void getAllAttachedSensors(){
  serial.print("getAllAttachedSensors");
  byte i;
  byte addr[8];
  byte cnt = 0;
  
  while(oneWire.search(addr)) {
    serial.print("sensor[" + String(cnt) + "] addr: \t");
    //read each byte in the address array
    for( i = 0; i < 8; i++) {
      serial.print("0x");
      if (addr[i] < 16) {
        serial.print('0');
      }
      // print each byte in the address array in hex format
      serial.print(addr[i], HEX);
      if (i < 7) {
        serial.print(" ");
      }else{
        serial.print("\n");
        cnt++;
      }
    }
    // a check to make sure that what we read is correct.
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        serial.print("CRC is not valid!\n");
        return;
    }
  }
  oneWire.reset_search();
  return;
}

void defineDeviceSensors(){
    //DeviceAddress -> typedef uint8_t DeviceAddress[8];
    // add values to sensor:    sensor_id  |  sensor_address  |  sensor_value;
    sensorsArr[0]   = (TSensor) {"100", {0x28, 0xff, 0x0c, 0x00, 0xa3, 0x16, 0x05, 0x02}, ""};
    sensorsArr[1]   = (TSensor) {"101", {0x28, 0xff, 0x69, 0x0c, 0xa3, 0x16, 0x03, 0x0e}, ""};  
}

void initWiFi_Server(){
  // Connecting to WiFi network
  serial.println();
  serial.print("Connecting to ");
  serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    serial.print(".");
  }
  serial.println ("");
  serial.println ("Connected to " + String(ssid));
  
  //prepare web server
  server.begin();
  serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);
  
  serial.print("device IP address: "); 
  serial.println(WiFi.localIP()); 
  serial.println("----------------------------------------------------------------------------------");
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

  float sensVal = 0;
  for (int i=0; i < sensorNum; i++){
    sensVal = readSensorDS18ByDeviceAddress(sensorsArr[i].sensor_address);
    if (!isnan(sensVal)){
      sensorsArr[i].sensor_value = String(sensVal);
    }
  }
}

void generateHttpPage(WiFiClient client){
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print("<p style='text-align: center;'>&nbsp;</p>");
  client.print("<p style='text-align: center;'><span style='font-size: x-large;'><strong>ESP8266 Web Server</strong></span></p>");
  client.print("<p style='text-align: center;'><span>temperature sa senzora</span></p>");
  
  client.print("<p style='text-align: center;'><span style='color: #0000ff;'><strong style='font-size: large;'>PEC_JEZGRA = ");
  client.println(String(sensorsArr[0].sensor_value));
  client.print("</strong></span><h style='text-align: center;'><span style='color: #0000ff;'><strong style='font-size: large;'><sup>o</sup>C</strong></span></h></p>");
   
  client.print("<p style='text-align: center;'><span style='color: #fb601c;'><strong style='font-size: large;'>PEC_POLAZ = ");
  client.println(String(sensorsArr[1].sensor_value));
  client.print("</strong></span><h style='text-align: center;'><span style='color: #fb601c;'><strong style='font-size: large;'><sup>o</sup>C</strong></span></h></p>");
  
  client.println("</html>");
}

// runs over and over again
void loop() {

  WiFiClient client = server.available();
  readSensors();
  generateHttpPage(client);
  
  // closing the client connection
  delay(REFRESH_TIMEOUT);
 
}   
