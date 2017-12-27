/*
 * WebSocketServer_LEDcontrol.ino
 *
 *  Created on: 26.11.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include "BasicUtils.h"

#define LED_RED     15
#define LED_GREEN   12
#define LED_BLUE    13

#define serial Serial

//WiFi
const char *ssid = "ProkletaKurva";
const char *password = "testnet123";

ESP8266WebServer server = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
    serial.println("webSocketEvent -> ###############################");
    switch(type) {
        case WStype_DISCONNECTED:
            serial.printf("[%u] Disconnected...!\n", num);
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

            if(payload[0] == '#') {
                // we get RGB data

                // decode rgb data
                uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

                analogWrite(LED_RED,    ((rgb >> 16) & 0xFF));
                analogWrite(LED_GREEN,  ((rgb >> 8) & 0xFF));
                analogWrite(LED_BLUE,   ((rgb >> 0) & 0xFF));
            }

            break;
        default:
          serial.println("webSocketEvent: type: UNKNOWN");
    }
}

void preparePINS(){
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    digitalWrite(LED_RED, 1);
    digitalWrite(LED_GREEN, 1);
    digitalWrite(LED_BLUE, 1); 
}

void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void setupWiFi_AP(){
    IPAddress address(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
  
    byte channel = 11;
    float wifiOutputPower = 20.5; //Max power
    WiFi.setOutputPower(wifiOutputPower);
    WiFi.setPhyMode(WIFI_PHY_MODE_11B);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    //C:\Users\spe\AppData\Roaming\Arduino15\packages\esp8266\hardware\esp8266\2.1.0\cores\esp8266\core_esp8266_phy.c
    //TRYING TO SET [114] = 3 in core_esp8266_phy.c 3 = init all rf
  
    WiFi.persistent(false);
    WiFi.softAPConfig(address, address, subnet);
    WiFi.softAP(ssid, password, channel);
}

//************************************************************************************
void setup() {
    serial.begin(115200);
    serial.setDebugOutput(true);
    delay(1000);

    serial.println("");
    serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    for(uint8_t t = 4; t > 0; t--) {
        serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        serial.flush();
        delay(1000);
    }

    serial.println("Configuring access point...");
    setupWiFi_AP();
    delay(100);
    
    IPAddress myIP = WiFi.softAPIP();
    serial.println("AP IP address: " + String(myIP[0]) + "." + String(myIP[1]) + "." + String(myIP[2]) + "." + String(myIP[3]));
   
    server.on("/", handleRoot);
    server.begin();
    
    serial.println("HTTP server started");

    //while(WiFi.waitForConnectResult() != WL_CONNECTED) {
    //    delay(100);
    //}

    serial.println(decodeWifiSTATUS(WiFi.status()));

    // start webSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    //if(MDNS.begin("esp8266")) {
    //    serial.println("MDNS responder started");
    //}

    // handle index
    server.on("/", []() {
        // send index.html
        server.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/></body></html>");
        serial.println("server -> handle index");
    });

    server.on("#", []() {
        server.send(200, "text/html", "<html><head></head><body>LED Test</body></html>");
        serial.println("server -> handle #");
    });

    server.begin();

    // Add service to MDNS
    //MDNS.addService("http", "tcp", 80);
    //MDNS.addService("ws", "tcp", 81);

    digitalWrite(LED_RED, 0);
    digitalWrite(LED_GREEN, 0);
    digitalWrite(LED_BLUE, 0);

}

void loop() {
    webSocket.loop();
    server.handleClient();
}

