/*
 * Mower Satellite
 *   Created on: 03.03.2017
 *   by DubyOriginal
 * 
 * ---------------------------------------------------------
 * -> Arduino Nano
 * -> RF433MHz (Receiver)
 * -> HC-SR04 (Ultrasound distance measurement) - PING
 * 
 * ---------------------------------------------------------
 * source: USB mini 5V:
 *  - Nano, HC-SR04, 
 *  - RF433 (RX) DC 5V
 */
 
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
#define SATELLITE_ID 1111

// PINS
//---------------------------------------------------------------
int echoPin = 4;    //Nano  (D4)  -> HCSR-04 (ECHO)
int trigPin = 5;    //Nano  (D5)  -> HCSR-04 (TRIG)
int LED_RF  = 6;    //Nano  (D6)  -> LED_RX
int LED_UZV = 7;    //Nano  (D7)  -> LED_UZV
//                  //Nano (D2)(INT0 / interupt 0) -> RF433 DATA (defined in mySwitch)
//---------------------------------------------------------------

//VAR
//---------------------------------------------------------------
long duration;
int distance;
unsigned long lastT = 0;
unsigned long timeDiff = 0;


void setup() {
  Serial.begin(115200);
  preparePINS();
  
  initialBlink();
  
  Serial.println("");
  Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("Mover Satellite - ID 1111");
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  Serial.println("READY");
  
}

void preparePINS(){
  pinMode(LED_RF, OUTPUT);
  pinMode(LED_UZV, OUTPUT);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
}

void pingUltraSound(){
  Serial.print("ping -> ");
  digitalWrite(LED_UZV, HIGH);

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration * 0.34 / 20;

  Serial.print("D: " + String(distance) + "cm");
  Serial.println(" (t: " + String(duration) + "us)");
  digitalWrite(LED_UZV, LOW);
}

void initialBlink(){
  digitalWrite(LED_RF, 1);
  digitalWrite(LED_UZV, 1);
  delay(1000);
  digitalWrite(LED_RF, 0);
  digitalWrite(LED_UZV, 0);
}

void printReceivedData(){
    Serial.print("\nReceived: ");
    Serial.print( mySwitch.getReceivedValue() );
    Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit / ");
    Serial.print("protocol: ");
    Serial.print(mySwitch.getReceivedProtocol());
    Serial.print(" / PW: ");
    Serial.print(mySwitch.getReceivedDelay()); 
    Serial.print("us /// ");
}

void loop() {
  digitalWrite(LED_RF, 0);
  if (mySwitch.available()) {
    digitalWrite(LED_RF, 1);
    
    int value = mySwitch.getReceivedValue();
    if (value == SATELLITE_ID) {
       timeDiff = micros() - lastT;
       if (timeDiff > 300000){
          printReceivedData();
          pingUltraSound();
          lastT = micros();
       }else {
          Serial.print("*");
       }

       //Serial.println("endT: " + String(micros()));
    
    }else {
      printReceivedData();
      Serial.println("unknown code: " + String(value));  
    }

     mySwitch.resetAvailable();  
  }
}
