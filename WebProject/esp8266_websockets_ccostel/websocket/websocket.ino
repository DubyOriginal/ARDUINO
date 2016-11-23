/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <WebSocketsServer.h>
#include <Hash.h>
#include <Metro.h> // Include the Metro library


Metro callBack = Metro(25);  // Instantiate an instanc
Metro printData = Metro(7000);

/* Set these to your desired credentials. */
const char *ssid = "TestNet";
const char *password = "test123";

WebSocketsServer webSocket = WebSocketsServer(81);
unsigned long lastTimeRefresh = 0;
int received =  0;

// WebSOcket Events 
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:{
        IPAddress ip = webSocket.remoteIP(num);
        Serial.print("WStype_CONNECTED: ");
        Serial.print("IP: ");
        Serial.println(ip);
      }
      break;
      
    case WStype_TEXT:
      { 
        String text = String((char *) &payload[0]);
        String mem = String(ESP.getFreeHeap());
        received++;
        /**
         * Stable config: 
         * WebsocketServer callBack Metro: 25 ms 
         * Client send rate: 10 ms.
         */
        if(callBack.check() == 1 && ESP.getFreeHeap() > 3288) {
           webSocket.sendTXT(num, "RAM: " + mem + "i = " + text + " ESPreceived = " + String(received) );
        }
        yield();
      }
      break;

    case WStype_BIN:
      hexdump(payload, length);
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
  }
}

void setup() {
  Serial.begin(115200);
//  Serial.println();
//  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
//  Serial.print("Configuring access point...");
  IPAddress address(10, 10, 100, 254);
  IPAddress subnet(255, 255, 255, 0);

  byte channel = 11;
  float wifiOutputPower = 20.5; //Max power
  WiFi.setOutputPower(wifiOutputPower);
  WiFi.setPhyMode(WIFI_PHY_MODE_11B);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);

  WiFi.persistent(false);
  WiFi.softAPConfig(address, address, subnet);
  WiFi.softAP(ssid, password, channel);
  IPAddress myIP = WiFi.softAPIP();

  Serial.print("AP IP address: ");
  Serial.println(myIP);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  if (printData.check() == 1) {
    Serial.println("received: " + String(received));
  }
  webSocket.loop();
}
