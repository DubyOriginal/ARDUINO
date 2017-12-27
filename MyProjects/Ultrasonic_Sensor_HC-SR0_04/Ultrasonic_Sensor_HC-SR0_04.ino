/* HC-SR04 Sensor
   The circuit:
  * VCC connection of the sensor attached to +5V
  * GND connection of the sensor attached to ground
  * TRIG connection of the sensor attached to digital pin 2
  * ECHO connection of the sensor attached to digital pin 4
  * Servo connected to pin 9
  */
 
#include <Servo.h> 
 
Servo servo;
 
static const int minAngle = 0;
static const int maxAngle = 180;
int angle=0;
int servoAngle;
int servoPos;

const int servoPin = 10;
const int echoPin = 7;
const int trigPin = 8;
 
void setup() {
  // initialize serial communication:
 Serial.begin(9600);
 
 servo.attach(servoPin);
 
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(servoPin, OUTPUT);

 Serial.println("init setup - done");
}
 
void loop()
{
  if (Serial.available()) {
    angle = Serial.parseInt();
    if(angle>-1){
      servoPos = constrain(map(angle, 0,180,minAngle,maxAngle),minAngle,maxAngle);
      servo.write(servoPin);
      getDistance();
    }
  }
}
 
void getDistance(){ 
  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  long duration, inches, cm;
 
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
 
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  duration = pulseIn(echoPin, HIGH);
 
  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);
 
  Serial.print(cm);
  Serial.println();
  
  delay(100);
}
 
long microsecondsToCentimeters(long microseconds){
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
