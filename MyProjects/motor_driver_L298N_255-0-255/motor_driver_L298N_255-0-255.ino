// connect motor controller pins to Arduino digital pins
// motor one
int enA = 10;
int in1 = 9;
int in2 = 8;

int outVal = 0;
int anIN = 0;

//******************************************************************************
void setup(){
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  // set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
}

//******************************************************************************
void myProgram(){
  int anIN = analogRead(A0);

  if (anIN < 512) {
    // left rotation
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);  
    outVal = map(anIN, 0, 512, 255, 0);
    displayVal(anIN);
    Serial.println("\t LEFT");
    
  }else{
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);  
    outVal = map(anIN, 512, 1023, 0, 255);
    displayVal(anIN);
    Serial.println("\t RIGHT");
  }

  analogWrite(enA, outVal);
}

//******************************************************************************
void displayVal(int anIN){
  // print the results to the serial monitor:
  Serial.print("anIN = ");
  Serial.print(anIN);
  Serial.print("\t motorA = ");
  Serial.print(outVal);
}

//******************************************************************************
void loop(){
  myProgram();
  delay(500);
}
