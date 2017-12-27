
// motorA
int enA = 10;
int in1 = 9;
int in2 = 8;

// motorB
int enB = 5;
int in3 = 6;  //zamjena pinoca zvog smjera vrtnje
int in4 = 7;

//params
int dSpeed = 85;

//******************************************************************************
void setup(){
  // motorA
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  // motorB
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
}

//******************************************************************************
void driveFroward(int ms){
  // turn on motor A
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, dSpeed);
  delay(ms);
}

//******************************************************************************
void driveBack(int ms){
  // turn on motor A
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, dSpeed);
  delay(ms);
}

//******************************************************************************
void rotateLeft(int ms){
  // turn on motor A
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, dSpeed);

  // turn on motor B
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, dSpeed);
  delay(ms);
}

//******************************************************************************
void turnOffMotor(int ms){
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  delay(ms);
}

//******************************************************************************
void loop(){
  turnOffMotor(2000);
  
  driveFroward(2200);
  
  rotateLeft(800);
 
}
