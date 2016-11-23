// ESC by Potentiometer
#include <Servo.h>
#include <SHT1x.h>

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  10
#define clockPin 11

SHT1x sht1x(dataPin, clockPin);
Servo myMotor;

int potVal;
int motorVal;
int oldVal = 54; //some random number

// Set everything up
//*****************************************************************************************************
//*****************************************************************************************************
void setup(){
  myMotor.attach(9);   // Put the motor to Arduino pin #9
  Serial.begin(38400);  // Required for I/O from Serial monitor
  Serial.println("initializing");  // Print a startup message
  
  delay(20);
  myMotor.write(0);  // Write to Servo
  delay(100);
}

//*****************************************************************************************************
//*****************************************************************************************************
void loop(){

    //ESC
    //**************************************************
    potVal = analogRead(0);    // read input PIN0
    motorVal = map( potVal, 0, 1023, 23, 130 );
    
    myMotor.write(motorVal);  // Write to Servo
    
    if (oldVal != motorVal){
          Serial.print("potVal: ");
          Serial.print(potVal);      // debug value - to make sure
          Serial.print(",  motorVal: ");
          Serial.println(motorVal);    // debug value - to make sure
          delay(100);
    }
    oldVal = motorVal;
 
 
   //SHT11
   //**************************************************
   float temp_c;
   float temp_f;
   float humidity;

   // Read values from the sensor
   //temp_c = sht1x.readTemperatureC();
   //temp_f = sht1x.readTemperatureF();
   humidity = sht1x.readHumidity();

   // Print the values to the serial port
   //Serial.print("Temperature: ");
   //Serial.print(temp_c, DEC);
   //Serial.print("C");
   //Serial.print(temp_f, DEC);
   //Serial.print("   Humidity: ");
   //Serial.print(humidity);
   //Serial.println("%");

   //delay(500);
}


