#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid     = "SkyNet";
const char* password = "adidasneo";
const char* serverIP   = "192.168.1.53";


float   tempVal = 0 ;
int     sizeHist = 100;        // Nombre de points dans l'historique - History size

unsigned long previousMillis = 0;  // Dernier point enregistré dans l'historique - time of last point added
const long intervalHist = 5000;    // Durée entre deux points dans l'historique

#define ONE_WIRE_BUS 2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

#define serial Serial
ESP8266WebServer server(80);

StaticJsonBuffer<10000> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
JsonArray& timestamp = root.createNestedArray("timestamp");
JsonArray& hist_t = root.createNestedArray("t");
 

void setup() {
  //Initialize serial and wait for port to open:
  serial.begin(115200);
  while (!serial) {;}

  WiFi.begin ( ssid, password );
  // Attente de la connexion au réseau WiFi / Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ); 
    serial.print ( "." );
  }
  // Connexion WiFi établie / WiFi connexion is OK
  serial.println ( "" );
  serial.print ( "Connected to " ); Serial.println ( ssid );
  serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );

  server.on("/mesures.json", sendMesures);

  //server.serveStatic("/js", SPIFFS, "/js");
  //server.serveStatic("/css", SPIFFS, "/css");

  server.begin();
  Serial.println ( "HTTP server started" );

}

float readSensor(){
    DS18B20.requestTemperatures(); 
    float tempVal = DS18B20.getTempCByIndex(0);
    Serial.println("Temperature: " + String(tempVal));
    return tempVal;
}

void sendMesures() {
  String json = "{\"t\":\"" + String(tempVal) + "\",";
  json += "\"h\":\"" + String(tempVal) + "\",";
  json += "\"pa\":\"" + String(tempVal) + "\"}";

  server.send(200, "application/json", json);
  serial.println("Mesures envoyees");
}

void sendHistory(){
  char json[10000];
  root.printTo(json, sizeof(json));

  //Serial.println(buffer);
  //String json = "{\"data\":\"[" + String(temp) +  

  server.send(200, "application/json", json);
  serial.println("data graph temp envoyees");
}


void addPtToHist(){
  unsigned long currentMillis = millis();

  if ( currentMillis - previousMillis > intervalHist ) {
     previousMillis = currentMillis;
     //timestamp.add(NTP.getTimeStr());
     hist_t.add(double_with_n_digits(tempVal, 1));
     if ( timestamp.size() > sizeHist ) {
      //Serial.println("erase old values");
      timestamp.removeAt(0);
      hist_t.removeAt(0);
     }
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  tempVal = readSensor();
  addPtToHist();
  delay(500);
}
