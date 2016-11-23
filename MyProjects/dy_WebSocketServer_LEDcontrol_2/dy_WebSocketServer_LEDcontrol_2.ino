#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

#define serial Serial
#define LED_RED     15
#define LED_GREEN   12
#define LED_BLUE    13

//Wifi
const char* ssid = "TestNet";
const char* password = "testnet123";
const int port = 8899;

boolean connectioWasAlive = true;
ESP8266WiFiMulti mWiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(81);

//*********************************************************************************************************************
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch (type) {
    case WStype_DISCONNECTED:
      serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      serial.printf("[%u] get Text: %s\n", num, payload);

      if (payload[0] == '#') {
        // we get RGB data

        // decode rgb data
        uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

        analogWrite(LED_RED,    ((rgb >> 16) & 0xFF));
        analogWrite(LED_GREEN,  ((rgb >> 8) & 0xFF));
        analogWrite(LED_BLUE,   ((rgb >> 0) & 0xFF));
      }
      break;
    case WStype_BIN:
      serial.printf("[%u] get binary lenght: %u\n", num, lenght);
      hexdump(payload, lenght);
    
      // send message to client
      // webSocket.sendBIN(num, payload, lenght);
    break;
  }

}

void preparePINS() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, 1);
  digitalWrite(LED_GREEN, 1);
  digitalWrite(LED_BLUE, 1);
}

void MDNSConnect() {  
  if (!MDNS.begin("YourHostName")) {
   serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  serial.println("mDNS responder started");
  MDNS.addService("ws", "tcp", 81);
  MDNS.addService("http", "tcp", 80);
}

//**********************************************************************************************************************
void setupWIFI() {
  serial.println("Configuring access point...");

  // Starting Wifi
  WiFi.hostname("NodeMCU");
  WiFi.mode(WIFI_STA);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(500);
  }


  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  serial.println("webSocket server - STARTED");

  // Add service to MDNS
  MDNSConnect();

}


//*********************************************************************************************************************
void setup() {
  serial.begin(115200);
  delay(1000);

  for (uint8_t t = 4; t > 0; t--) {
    serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    serial.flush();
    delay(1000);
  }

  preparePINS();

  setupWIFI();
  /*
    mWiFiMulti.addAP(ssid, password);
    delay(2000);

    while(mWiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }
    
    monitorWiFi();
  
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    serial.println("webSocket - initialized");
    */
}


//*********************************************************************************************************************
void loop() {
  webSocket.loop();
}





