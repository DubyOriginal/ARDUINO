/*
 * Mower Base Controller
 *   Created on: 26.11.2016
 *   by DubyOriginal
 *  
 * ---------------------------------------------------------  
 * -> NodeMCU - ESP8266 WiFi
 * -> RF433MHz (Transmiter)
 * -> HC-SR04 (Ultrasound distance measurement) - PONG
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

#include <RCSwitch.h>   // for RF 433MHz transmiter module

//---------------------------------------------------------------
#define SATELLITE1_ID 1111
#define SATELLITE2_ID 2222

// PINS (defined in BasicUtils)
//---------------------------------------------------------------
int RF_T1Data = D6;  
int LED_UZV   = TX;   

int echoPin = D2;  
int trigPin = D1;   
int analogPin = A0;    

// motorA
int enA = D0;
int in1 = D5;
int in2 = D8;
// motorB
int in3 = D3;  
int in4 = D4;
int enB = D5;

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
unsigned long t1 = 0;
unsigned long t2 = 0;
unsigned long t3 = 0;
unsigned long t4 = 0;
int dSpeed = 120;        //initial value
int driveState = STOP;   //initial value

//OTHER
//---------------------------------------------------------------
#define serial Serial
RCSwitch switchRFT1 = RCSwitch();
WebSocketsServer webSocket = WebSocketsServer(81);

//**************************************************************************************************************************************
//**************************************************************************************************************************************
void setup() {
    serial.begin(115200);
    serial.setDebugOutput(false);

    preparePINS();
    driveStop();  
    initialBlink();
    initRF433Driver();

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
    pinMode(analogPin, INPUT);   // Sets the analogPin as an Input
    pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
    pinMode(LED_UZV, OUTPUT);

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
    digitalWrite(LED_UZV, 1);
    delay(1000);
    digitalWrite(LED_UZV, 0);
}

void initRF433Driver(){
  // Transmitter is connected to Arduino Pin #12  
  switchRFT1.enableTransmit(RF_T1Data);
  
  // Optional set protocol (default is 1, will work for most outlets)
  // switchRFT1.setProtocol(2);

  // Optional set pulse length.
  // switchRFT1.setPulseLength(320);
  
  // Optional set number of transmission repetitions.
  switchRFT1.setRepeatTransmit(5);
   
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
  if (msgCode=="CMD_LED"){
    serial.println("execute LED event");
    resultStr = "RES_LED";
    
  }else if (msgCode == "CMD_ANALOG"){
    serial.println("execute ANALOG event");
    String analogValue = readAnalog();
    resultStr = "RES_ANALOG," + analogValue;

  }else if (msgCode == "CMD_HCSR04"){
    serial.println("execute CMD_HCSR04 event");
    String sensorValue = readHCSR04();
    resultStr = "RES_HCSR04," + sensorValue;

  }else if (msgCode == "CMD_RFT1"){
    serial.println("execute CMD_RFT1 event");
    String responseValue = sendMsgRFT1();
    resultStr = "RES_RFT1," + responseValue;

  }else if (msgCode == "CMD_PINGPONG"){
    serial.println("execute CMD_PINGPONG event");
    String responseValue = triggerPingPong();
    resultStr = "RES_PINGPONG," + responseValue;

//DRIVE
//---------------------------------------------------
  }else if (msgCode.startsWith("D_MANUAL")){
    serial.println("execute D_MANUAL event");
    String responseValue = prepareAndRunManualDrive(msgCode);
    resultStr = "RES_MANUAL," + responseValue;

  }else if (msgCode == "D_FORWARD"){
    serial.println("execute D_FORWARD event");
    String responseValue = triggerPingPong();
    resultStr = "RES_FORWARD," + responseValue;

  }else if (msgCode == "D_BACK"){
    serial.println("execute D_BACK event");
    String responseValue = triggerPingPong();
    resultStr = "RES_BACK," + responseValue;

  }else if (msgCode == "D_ROTLEFT"){
    serial.println("execute D_ROTLEFT event");
    String responseValue = triggerPingPong();
    resultStr = "RES_ROTLEFT," + responseValue;

  }else if (msgCode == "D_ROTRIGHT"){
    serial.println("execute D_ROTRIGHT event");
    String responseValue = triggerPingPong();
    resultStr = "RES_ROTRIGHT," + responseValue;

  }else if (msgCode == "D_STOP"){
    serial.println("execute D_STOP event");
    String responseValue = triggerPingPong();
    resultStr = "RES_STOP," + responseValue;
    
  }else if (msgCode == "D_SPEED"){
    serial.println("execute D_SPEED event");
    String responseValue = triggerPingPong();
    resultStr = "RES_SPEED," + responseValue;
  }else {
    serial.println("unknown message code");
  }
  return resultStr;
}

//**************************************************************************************************************************************
//**************************************************************************************************************************************

//potenciometer
//*******************************************************
String readAnalog() {
  Serial.print("readAnalog -> ");
  int analog = analogRead(analogPin);    // read the input pin
  String analogStr = String(analog);
  Serial.println(analogStr);
  return analogStr;
}

//RF T1 broadcast message
//*******************************************************
String sendMsgRFT1() {
  Serial.println("sendMsgRFT1");
  switchRFT1.send(SATELLITE1_ID, 24);  
  return "SENT";
}

//TRIGGER PING-PONG
//Base (PING), Sattelite (PONG)
//*******************************************************
String triggerPingPong() {
  Serial.println("triggerPingPong");
  int calibDelay = analogRead(analogPin);    // read A0 (0,415ms)
  calibDelay = 226852 + (calibDelay * 10); 

  t0 = micros();
  //send msg to SATELLITE to start PONG
  switchRFT1.send(SATELLITE1_ID, 24);   //225ms

  t1 = micros();
  Serial.println("send2RF: " + String(t1-t0) + "us");
  
  delayMicroseconds(calibDelay);  
  Serial.println("calibDelay: " + String(calibDelay) + "us");

  t2 = micros();
  //Serial.println("delayTime: " + String(t2-t1) + "us");

  //initiate BASE PING
  String result = readHCSR04();  //1,3ms

  t3 = micros();
  Serial.println("readHCSR04Time: " + String(t3-t2) + "us");
   
  return result;
}

//UltraSound sensor
//*******************************************************
String readHCSR04() {
  Serial.print("readHCSR04 -> ");
  digitalWrite(LED_UZV, HIGH);
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration * 0.34 / 20;

  Serial.print("s = " + String(distance) + " cm");
  Serial.print(" (t = ");
  Serial.print(duration); // Sends the distance value into the Serial Port
  Serial.println(" us)");

  digitalWrite(LED_UZV, LOW);
  String disStr = String(distance);
  return disStr;

}

//**************************************************************************************************************************************
//**************************************************************************************************************************************

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
void updateSpeed(){
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

//**************************************************************************************************************************************
//**************************************************************************************************************************************
void loop() {
    webSocket.loop();
    //server.handleClient();
}



