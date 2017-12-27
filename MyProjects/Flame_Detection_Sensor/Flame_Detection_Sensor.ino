// ***************************************************************************************
// Flame Sensor Analog example 
// ***  by Duby  ***
// ***************************************************************************************

/*
PINOUT:
  - A0 -> Arduino A0 (0-1023)
  - D0 -> 
*/

int analogOutPin = 3;           // PWM

int sensorReading = 0;        // value read from the pot
int outputValue = 0;          // value output to the PWM (analog out)

int sensorRead_Dark = 1017;        //value in full darkness
int sensorRead_Daylight = 680;     //value daylight value range 600-900 
int sensorRead_Fire = 20;          //value with close fire

void setup() {
  // initialize serial communication @ 9600 baud:
  Serial.begin(9600);  
}
void loop() {
  
  // read the sensor on analog A0:
  // ***************************************************************************************
  sensorReading = analogRead(A0);
  delay(50);  // delay between reads
  
  Serial.print("sensorReading: ");
  Serial.println(sensorReading);

  // adjust AnalogIN for AnalogOUT
  // ***************************************************************************************
  outputValue = map(sensorReading, 0, sensorRead_Daylight, 255, 0);  
  if (outputValue < 0) outputValue = 0;
  // change the analog out value:
  analogWrite(analogOutPin, outputValue);
  //Serial.println(outputValue);

  // map the sensor range for Analog Display
  // ***************************************************************************************
  /*
  int range = map(sensorReading, 0, 1024, 8, 0);
  switch (range) {
  case 0:    
    Serial.println("*");
    break;
  case 1:   
    Serial.println("**");
    break;
  case 2:  
    Serial.println("***");  
    break;
  case 3:  
    Serial.println("****");  
    break;
  case 4:  
    Serial.println("*****");  
    break;
  case 5:  
    Serial.println("******");  
    break;
  case 6:  
    Serial.println("*******");  
    break;
  case 7:  
    Serial.println("********");  
    break;
  case 8:  
    Serial.println("*********");  
    break;
  }
  
  */
  
}
