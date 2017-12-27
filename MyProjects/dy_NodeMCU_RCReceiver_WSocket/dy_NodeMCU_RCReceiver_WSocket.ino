/*
 * Module for Mobile & RC Receiver 6CH communication
 *   Created on: 28.04.2017
 *   by DubyOriginal
 *  
 * ---------------------------------------------------------  
 * -> NodeMCU - ESP8266 WiFi
 * -> FlySky Receiver 6CH
 * 
 * --------------------------------------------------------- 
 * source: USB mini 3.3V
 * 
 * ---------------------------------------------------------
 * Programmer setup:
 *    - Tools -> Board -> Generic ESP8266 Module
 *    - CPU 80MHz, Flash 40MHz
 *    - Flash size: 4MB
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include "BasicUtils.h"


// PINS (defined in BasicUtils)
//---------------------------------------------------------------  
//const int analogPin = A0;
//const int B_LED = LED_BUILTIN;
const int pinCH1 = D0;
const int pinCH2 = D1;
const int pinCH3 = D2;    
const int pinCH4 = D3;
const int pinCH5 = D4; 

// CONST
//---------------------------------------------------------------
//WiFi
const char *ssid = "MowerNet";
const char *password = "mower123";

// VAR
//---------------------------------------------------------------

//OTHER
//---------------------------------------------------------------
#define serial Serial
WebSocketsServer webSocket = WebSocketsServer(81);

//**************************************************************************************************************************************
//**************************************************************************************************************************************
void setup() {
    serial.begin(115200);
    serial.setDebugOutput(false);

    preparePINS();
    //initialBlink();

    serial.println("");
    serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    serial.println("Configuring access point...");
    setupWiFi_AP();
    delay(100);
    
    IPAddress myIP = WiFi.softAPIP();
    serial.println("AP IP address: " + String(myIP[0]) + "." + String(myIP[1]) + "." + String(myIP[2]) + "." + String(myIP[3]));
    serial.println("AP IP address: " + myIP.toString());
    serial.println(decodeWifiSTATUS(WiFi.status()));

    // start webSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    serial.println("WebSocket Server Started");

}


//*********************************************************************************************************
void preparePINS(){
    //pinMode(B_LED, OUTPUT); 
    pinMode(pinCH1, INPUT);   
    pinMode(pinCH2, INPUT);
    pinMode(pinCH3, INPUT);
    pinMode(pinCH4, INPUT);
    pinMode(pinCH5, INPUT);
}

/*
void initialBlink(){
    digitalWrite(B_LED, 1);
    delay(700);
    digitalWrite(B_LED, 0);
    delay(700);
    digitalWrite(B_LED, 1);
    delay(700);
    digitalWrite(B_LED, 0);
    delay(700);
    digitalWrite(B_LED, 1);
    delay(700);
    digitalWrite(B_LED, 0);
}*/

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

//**************************************************************************************************************************************
//**************************************************************************************************************************************
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t sizeT) {
    serial.println("webSocketEvent");
    serial.println("-------------------S-------------------");
    //serial.printf("num: %d\n", num);         //0
    //serial.printf("type: %d\n", type);       //2, 3,...
    //serial.printf("payload: %s\n", payload);   //text
    //serial.printf("sizeT: %u\n", sizeT);     //text length
    
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
        case WStype_TEXT: {
            //serial.printf("[%u] get Text: %s\n", num, payload);

            //webSocket.sendTXT(num, payload, lenght);     // echo data back to browser
            //webSocket.broadcastTXT("1234567890123456789012345678901234567890", 30);   // send data to all connected clients

            String msgCode = String((char *) &payload[0]);
            String resultStr = handleWSEventForCode(msgCode);

            webSocket.sendTXT(num, resultStr);     // echo data back
            //webSocket.broadcastTXT(resultStr); // send data to all connected clients
        }
            break;
        case WStype_BIN:
            serial.printf("[%u] get binary length: %u\n", num, sizeT);
            hexdump(payload, sizeT);
            // echo data back to browser
            webSocket.sendBIN(num, payload, sizeT);
            break;
        case WStype_ERROR:
            serial.println("webSocketEvent - ERROR");
            break;
        default:
          serial.println("webSocketEvent - UNKNOWN");
    }
    serial.println("-------------------E-------------------");
}

//*********************************************************************************************************
String handleWSEventForCode(String msgCode){
  serial.println("handleWSEventForCode - " + msgCode);
  String resultStr = "";
  if (msgCode=="CMD_LED"){
    serial.println("execute LED event");
    resultStr = "RES_LED";
  //---------------------------------------------------
  }else if (msgCode.startsWith("D_")){
    serial.println("execute D_MANUAL event");
    String responseValue = forwardCMD(msgCode);
    resultStr = "RES_FORWARDED," + responseValue;
    
  }else {
    serial.println("unknown message code");
  }
  return resultStr;
}

//*******************************************************
String readAnalog() {
  Serial.print("readAnalog -> ");
  //int analog = analogRead(analogPin);    // read the input pin
  //String analogStr = String(analog);
  //Serial.println(analogStr);
  //return analogStr;
  return "1024";
}

//**************************************************************************************************************************************
//**************************************************************************************************************************************
void listenReceiverEvent(){
  int ch1;
  int ch2;
  int ch3;
  int ch4;
  int ch5;
  int CHANNEL[6];

  int ch1RAW = 0;
  int ch2RAW = 0;
  int ch3RAW = 0;
  int ch4RAW = 0;
  int ch5RAW = 0;
  
  // Read the pulse width of each channel
  ch1RAW = pulseIn(pinCH1, HIGH, 30000); 
  ch2RAW = pulseIn(pinCH2, HIGH, 30000);  
  ch3RAW = pulseIn(pinCH3, HIGH, 30000);
  ch4RAW = pulseIn(pinCH4, HIGH, 30000);
  ch5RAW = pulseIn(pinCH5, HIGH, 30000);

  //Serial.println("RAW CH 1:" + String(ch1RAW) + ",\t CH 2:" + String(ch2RAW) + ",\tCH 3:" + String(ch3RAW) + ",\tCH 4:" + String(ch4RAW) + ",\tCH 5:" + String(ch5RAW));

  //move = map(ch2, 1000,2000, -500, 500); //center over zero
  ch1 = map(ch1RAW, 985, 1995, 0, 1023);
  ch2 = map(ch2RAW, 985, 1995, 0, 1023);
  ch3 = map(ch3RAW, 985, 1995, 0, 1023);
  ch4 = map(ch4RAW, 985, 1995, 0, 1023);
  ch5 = map(ch5RAW, 985, 1995, 0, 1023);

    int offset = 0;
  CHANNEL[1] = ch1 - offset;
  CHANNEL[2] = ch2 - offset;
  CHANNEL[3] = ch3 - offset;
  CHANNEL[4] = ch4 - offset;
  CHANNEL[5] = ch5 - offset;

  String chStr1 = "NC";
  String chStr2 = "NC";
  String chStr3 = "NC";
  String chStr4 = "NC";
  String chStr5 = "NC";
  
  if (CHANNEL[1] > 0){chStr1 = String(CHANNEL[1]);} 
  if (CHANNEL[2] > 0){chStr2 = String(CHANNEL[2]);}
  if (CHANNEL[3] > 0){chStr3 = String(CHANNEL[3]);}
  if (CHANNEL[4] > 0){chStr4 = String(CHANNEL[4]);}
  if (CHANNEL[5] > 0){chStr5 = String(CHANNEL[5]);}

  Serial.println("CH 1:" + chStr1 + ",\t CH 2:" + chStr2 + ",\tCH 3:" + chStr3 + ",\tCH 4:" + chStr4 + ",\tCH 5:" + chStr5);

  //encodeChannelValues(CHANNEL);

  delay(50); 
}
//**************************************************************************************************************************************
//**************************************************************************************************************************************

String forwardCMD(String params){
    serial.println("FORWARD CMD: " + params);
    return "OK";
}

//**************************************************************************************************************************************
//**************************************************************************************************************************************
void loop() {
    webSocket.loop();

    listenReceiverEvent();
}



