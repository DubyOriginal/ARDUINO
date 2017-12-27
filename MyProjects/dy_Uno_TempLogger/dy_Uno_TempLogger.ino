#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//AP definitions - update this
#define AP_SSID     "SkyNet"
#define AP_PASSWORD "adidasneo"

// EasyIoT Cloud definitions - change EIOT_CLOUD_INSTANCE_PARAM_ID
#define EIOT_CLOUD_INSTANCE_PARAM_ID    "59bcdbda47976c5b6bce7fa7/XbmOLNIhsvj9QXJV"
#define REPORT_INTERVAL 60 // in sec


#define EIOT_CLOUD_ADDRESS     "cloud.iot-playground.com"
#define EIOT_CLOUD_PORT        40404




#define ONE_WIRE_BUS 2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


float oldTemp;


void setup() {
  Serial.begin(115200);
  
  wifiConnect();
    
  oldTemp = -1;
}

void loop() {
  float temp;
  Serial.println("*"); 

  do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    Serial.print("Temperature: ");
    Serial.println(temp);
  } while (temp == 85.0 || temp == (-127.0));
  
  if (temp != oldTemp){
    sendTeperature(temp);
    oldTemp = temp;
  }
  
  int cnt = REPORT_INTERVAL;
  
  while(cnt--)
    delay(1000);
}

void wifiConnect(){
    Serial.print("Connecting to AP");
    WiFi.begin(AP_SSID, AP_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }

  Serial.println("");
  Serial.println("WiFi connected");  
}

void sendTeperature(float temp){ 
  Serial.println("sendTeperature"); 
  WiFiClient client;
   
  while(!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT)) {
   Serial.println("connection failed");
   wifiConnect(); 
  }

  String url = "";
  // URL: "/RestApi/SetParameter/[insatnce id]/[parameter id]/[value]
  url += "/RestApi/SetParameter/"+ String(EIOT_CLOUD_INSTANCE_PARAM_ID) + "/"+String(temp); // generate EasIoT cloud update parameter URL

  Serial.print("POST data to URL: ");
  Serial.println(url);
  
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" + 
               "Connection: close\r\n" + 
               "Content-Length: 0\r\n" + 
               "\r\n");

  delay(100);
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("Connection closed");
}

