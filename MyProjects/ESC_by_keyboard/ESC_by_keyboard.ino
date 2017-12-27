#include <Servo.h>

Servo myMotor;
String inputValue;

// Set everything up
//***********************************************************
void setup(){
  myMotor.attach(9);   // Put the motor to Arduino pin #9
  Serial.begin(9600);  // Required for I/O from Serial monitor
  Serial.println("initializing");  // Print a startup message
}

//***********************************************************
void loop()
{
  // If there is incoming value
  if(Serial.available() > 0) {
    
    char ch = Serial.read();  // read the value
 
    //If ch isn't a newline (linefeed) character, we will add the character to the inputValue
    if (ch != 10){
      // Print out the value received so that we can see what is happening
      Serial.print("I have received: ");
      Serial.print(ch, DEC);
      Serial.print('\n');
   
      // Add the character to the inputValue
      inputValue += ch;
    }
    // received a newline (linefeed) character this means we are done making a string
    else{
      // print the incoming string
      Serial.println("I am printing the entire string");
      Serial.println(inputValue);
   
      // Convert the string to an integer
      int val = inputValue.toInt();
   
      // print the integer
      Serial.println("Printing the value: ");
      Serial.println(val);
   
      // We only want to write an integer between 0 and 180 to the motor.
      if (val > -1 && val < 181){
       // Print confirmation that the value is between 0 and 180
       Serial.println("Value is between 0 and 180");
       myMotor.write(val);  // Write to Servo
     }
     
     // The value is not between 0 and 180. We do not want write this value to the motor.
     else{
       Serial.println("Value is NOT between 0 and 180");
       Serial.println("Error with the input");
     }
     
      inputValue = "";  // Reset inputValue
    }
  }
}
