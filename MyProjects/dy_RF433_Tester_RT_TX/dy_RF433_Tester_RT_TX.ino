/*
 * RF433 Tester
 *   Created on: 03.03.2017
 *   by DubyOriginal
 * 
 * ---------------------------------------------------------
 * -> Arduino Nano
 * -> RF433MHz (Transmiter / Receiver)
 * 
 * ---------------------------------------------------------
 * source: USB mini 5V:
 *  - Nano, HC-SR04, 
 *  - RF433 (MX-05V -> RX) 5V
 *  - RF433 (MX-FS-03V -> TX) 3.5-12V  
 */

#include <Arduino.h>

//###############################################################
int MODE = 1;         //0->RECEIVER, 1-> TRANSMITTER

// for RF 433MHz transmiter module
//---------------------------------------------------------------
#include <RCSwitch.h>
RCSwitch switchRFT = RCSwitch();
#define SATELLITE_ID 1111

//PINS
//---------------------------------------------------------------
int TX_DATA = 5;    //Nano  (D5)  -> RF433 TX data     
int TX_LED  = 3;    //Nano  (D3)  -> LED_TX
int RX_LED  = 4;    //Nano  (D4)  -> LED_RX
int BTN_PIN = 6;    //Nano  (D6)  -> 0 -> ¤----| R1 |----¤ GND // (1 -> VCC (D6)----| R1 |----¤ GND)
//int RX_DATA = 2;  //Nano  (D2)(INT0 / interupt 0) -> RF433 DATA (switchRFT.enableReceive(0))

//VAR
//---------------------------------------------------------------
unsigned long lastT = 0;
unsigned long timeDiff = 0;
int btnState = 0;     // variable for reading the pushbutton status

//******************************************************************************************************
void setup() {
  Serial.begin(115200);
  preparePINS();
  initRF433Driver();
  
  Serial.println("");
  Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("Mover RF Tester - ID 2222");

  initialBlink();

  Serial.println("device ready");
}

//*********************************************************************************************************
void preparePINS(){
    pinMode(RX_LED, OUTPUT);
    pinMode(TX_LED, OUTPUT);

    // initialize the pushbutton pin as an input:
    pinMode(BTN_PIN, INPUT);

}

void initialBlink(){
  digitalWrite(RX_LED, 1);
  digitalWrite(TX_LED, 1); 
  delay(1000);
  digitalWrite(RX_LED, 0);
  digitalWrite(TX_LED, 0); 
}

void initRF433Driver(){
  
  //TRANSMITER
  //--------------------------------------------------------------
  if (MODE == 1){
    switchRFT.enableTransmit(TX_DATA);  // Transmitter is connected to Arduino TX_DATA
    switchRFT.setProtocol(1);           // Optional set protocol (default is 1, will work for most outlets)
    switchRFT.setPulseLength(350);      // Optional set pulse length.
    switchRFT.setRepeatTransmit(20);     // Optional set number of transmission repetitions.

  //RECEIVER
  //--------------------------------------------------------------
  }else if (MODE == 0){
    switchRFT.enableReceive(0);  // Receiver on interrupt 0 => that is pin D2
  }
}

//RF T1 broadcast message
//*******************************************************
String sendMsgRFT() {
  Serial.println("sendMsgRFT");
  switchRFT.send(SATELLITE_ID, 24);  
  return "RES_RFT,SENT";
}

void printReceivedData(){
    Serial.print("\nReceived: ");
    Serial.print( switchRFT.getReceivedValue() );
    Serial.print(" / ");
    Serial.print( switchRFT.getReceivedBitlength() );
    Serial.print("bit / ");
    Serial.print("protocol: ");
    Serial.print(switchRFT.getReceivedProtocol());
    Serial.print(" / PW: ");
    Serial.print(switchRFT.getReceivedDelay()); 
    Serial.print("us /// ");
}

//******************************************************************************************************
void loop() {

  //TRANSMITER
  //--------------------------------------------------------------
  if (MODE == 1){
      btnState = digitalRead(BTN_PIN);
      if (btnState == HIGH) {
        digitalWrite(TX_LED, 1); 
        String result = sendMsgRFT();
        Serial.println(result);
        delay(400);
        digitalWrite(TX_LED, 0); 
      }

  //RECEIVER
  //--------------------------------------------------------------
  }else if (MODE == 0){
    digitalWrite(RX_LED, 0);
    if (switchRFT.available()) {
      digitalWrite(RX_LED, 1);
      
      int value = switchRFT.getReceivedValue();
      if (value == SATELLITE_ID) {
         timeDiff = micros() - lastT;
         if (timeDiff > 300000){
            printReceivedData();
            lastT = micros();
         }else {
            Serial.print("*");
         }
      
      }else {
        printReceivedData();
        Serial.println("unknown code: " + String(value));  
      }
  
       switchRFT.resetAvailable();  
    }
  }
}
