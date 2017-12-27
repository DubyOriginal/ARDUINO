/*
 * Mower Base Controller 
 *   Driving Module + Web Socket
 *   Created on: 17.03.2017
 *   by DubyOriginal
 *  
 * ---------------------------------------------------------  
 * -> NodeMCU - ESP8266 WiFi
 * -> L298N - motor driver
 * 
 * --------------------------------------------------------- 
 * source: Battery
 * -> L298N 5V -> NodeMCU Vin pin
 * 
 * ---------------------------------------------------------
 * Programmer setup:
 *    - Tools -> Board -> Generic ESP8266 Module
 *    - CPU 80MHz, Flash 40MHz
 *    - Flash size: 4MB
 */

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include "BasicUtils.h"

//---------------------------------------------------------------  
int LED = 1;  //gpio1 - TX (onboard led)

// motorA
int enA = D0;   //zelena
int in1 = D1;   //crvena
int in2 = D2;   //ljub.
// motorB
int in3 = D3;   //smeđa
int in4 = D4;   //plava
int enB = D5;   //žuta

// CONST
//---------------------------------------------------------------
//WiFi
const char *ssid = "MowerNet";
const char *password = "mower123";
//Drive
const int FORWARD = 0;
const int BACK    = 1;
const int RLEFT   = 2;
const int RRIGHT  = 3;
const int STOP    = 4;

//VAR
//---------------------------------------------------------------
long duration;
int distance;
unsigned long t0 = 0;
//initial value
int driveState = STOP;   //initial value
int dSpeed = 120;        //initial value

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
    driveStop();  
    initialBlink();

    serial.println("");
    serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
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
    serial.println("AP IP address: " + myIP.toString());
    serial.println(decodeWifiSTATUS(WiFi.status()));

    // start webSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    serial.println("WebSocket Server Started");

}


//*********************************************************************************************************
void preparePINS(){
    //pinMode(LED, OUTPUT);
  
    //drive
    //----------------------------
    // motorA
    pinMode(enA, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    // motorB
    pinMode(enB, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
}

void initialBlink(){
    digitalWrite(LED, 1);
    delay(1000);
    digitalWrite(LED, 0);
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
  if (msgCode=="CMD_TEST"){
    serial.println("TEST event");
    resultStr = "RES_TEST,OK";

//DRIVE
//---------------------------------------------------
  }else if (msgCode.startsWith("D_MANUAL")){
    serial.println("execute D_MANUAL event");
    String responseValue = prepareAndRunManualDrive(msgCode);
    resultStr = "RES_MANUAL," + responseValue;

  }else if (msgCode == "D_FORWARD"){
    serial.println("execute D_FORWARD event");
    driveFroward();
    resultStr = "RES_FORWARD,OK";

  }else if (msgCode == "D_BACK"){
    serial.println("execute D_BACK event");
    driveBack();
    resultStr = "RES_BACK,OK";

  }else if (msgCode == "D_ROTLEFT"){
    serial.println("execute D_ROTLEFT event");
    rotateLeft();
    resultStr = "RES_ROTLEFT,OK";

  }else if (msgCode == "D_ROTRIGHT"){
    serial.println("execute D_ROTRIGHT event");
    rotateRight();
    resultStr = "RES_ROTRIGHT,OK";

  }else if (msgCode == "D_STOP"){
    serial.println("execute D_STOP event");
    driveStop();
    resultStr = "RES_STOP,OK";
    
  }else if (msgCode == "D_SPEED"){
    serial.println("execute D_SPEED event");
    updateSpeed(msgCode);
    resultStr = "RES_SPEED,OK";
  }else {
    serial.println("unknown message code");
  }
  return resultStr;
}

//**************************************************************************************************************************************
//**************************************************************************************************************************************

//******************************************************************************
String prepareAndRunManualDrive(String params){
    //params: "D_MANUAL,-125,320"
    int comma1Index = params.indexOf(',');
    int comma2Index = params.indexOf(',', comma1Index + 1);  //  Search for the next comma just after the first

    String firstValue = params.substring(0, comma1Index);
    String motAStr = params.substring(comma1Index + 1, comma2Index);
    String motBStr = params.substring(comma2Index + 1); // To the end of the string

    Serial.println(String("motorA: " +  motAStr + ", motorB: " + motBStr));
    int motA = motAStr.toInt();
    int motB = motBStr.toInt();
    manualDrive(motA, motB);
    return "OK";
}

//MANUAL DRIVE CONTROL
//*********************************************************************************************************
void manualDrive(int mSpeedA, int mSpeedB){
  //MOTOR A
  //*********************************************
  if (mSpeedA < 0){
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    mSpeedA = -1 * mSpeedA;
    
  }else if (mSpeedA > 0){
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  
  }else{
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
  analogWrite(enA, mSpeedA);

  //MOTOR B
  //*********************************************
  if (mSpeedB < 0){
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    mSpeedB = -1 * mSpeedB;
    
  }else if (mSpeedB > 0){
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  
  }else{
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
  }
  analogWrite(enB, mSpeedB);

}

//SMART DRIVE CONTROL
//*********************************************************************************************************
void driveFroward(){
  Serial.print("drive: FORWARD (");
  Serial.print(dSpeed);
  Serial.println(")");
  driveState = FORWARD;
 
  // turn on motor A
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, dSpeed);
}

//******************************************************************************
void driveBack(){
  Serial.print("drive: BACK (");
  Serial.print(dSpeed);
  Serial.println(")");
  driveState = BACK;
  
  // turn on motor A
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, dSpeed);
}

//******************************************************************************
void rotateLeft(){
  Serial.print("drive: RLEFT (");
  Serial.print(dSpeed);
  Serial.println(")");
  driveState = RLEFT;
  
  // turn on motor A
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, dSpeed);
}

//******************************************************************************
void rotateRight(){
  Serial.print("drive: RRIGHT (");
  Serial.print(dSpeed);
  Serial.println(")");
  driveState = RRIGHT;
  
  // turn on motor A
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, dSpeed);
}

//******************************************************************************
void driveStop(){
  Serial.println("drive: STOP");
  driveState = STOP;
  
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  analogWrite(enA, 0);
  analogWrite(enB, 0);
}

//******************************************************************************
void updateSpeed(String params){
    
    //params: "D_SPEED,620"
    int paramsLenght = params.length();
    int comma1Index = params.indexOf(',');
    String firstValue = params.substring(0, comma1Index);    //D_SPEED
    String dSpeedStr = params.substring(comma1Index + 1, paramsLenght);   //620
    dSpeed = dSpeedStr.toInt();
    Serial.println("updateSpeed: " +  dSpeedStr);
    
    if (driveState = FORWARD){
        driveFroward();
    } else if (driveState = BACK){
        driveBack();
    } else if (driveState = RLEFT){
        rotateLeft();
    } else if (driveState = RRIGHT){
        rotateRight();
    }
}



//**************************************************************************************************************************************
//**************************************************************************************************************************************
void loop() {
    webSocket.loop();
    
}



