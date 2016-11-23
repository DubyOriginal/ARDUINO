#include <ESP8266WiFi.h>

//CONST
//***********************************************
//const char* ssid = "SkyNet";
//const char* password = "adidasneo";

const char* ssid = "mower";
const char* password = "mower123";

const int FORWARD = 0;
const int BACK    = 1;
const int RLEFT   = 2;
const int RRIGHT  = 3;
const int STOP    = 4;

// motorA
int enA = D0;
int in1 = D1;
int in2 = D2;

// motorB
int in3 = D3;  
int in4 = D4;
int enB = D5;

//VAR
//***********************************************
int dSpeed = 120;          //initial value
int driveState = STOP;     //initial value
unsigned long startT = 0;

// Create an instance of the server, specify the port to listen on as an argument
WiFiServer server(80);
WiFiClient client;

//*********************************************************************************************************
//*********************************************************************************************************
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("");
  Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("MowerApp v1.1.10");

  preparePINS();

  delay(1000);

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected");

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  Serial.println("---------------------------------------------------------------------");
 
}

//*********************************************************************************************************
//*********************************************************************************************************
void preparePINS(){
  //pinMode(2, OUTPUT);
  //digitalWrite(2, 0);

  // motorA
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  // motorB
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  stopMotors();
}

void test(){
  String req = "GET /cmd/speed/248 HTTP/1.1";
  Serial.print("req.length: ");
  Serial.println(req.length());

  Serial.print("req.indexOf(/cmd/speed/): ");
  Serial.println(req.indexOf("/cmd/speed/"));
  
  String result = req.substring(15, 18);
  Serial.print("result: ");
  Serial.println(result);
}

//MANUAL DRIVE CONTROL
//*********************************************************************************************************
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
void stopMotors(){
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
void prepareAndRunManualDrive(String req){
    //req = "GET /cmd/drive/-254,-262 HTTP/1.1"
    int lenCmd1 = 11; //  "/cmd/drive/"
    int lenCmd2 = 9;  //  " HTTP/1.1"
    int posVal = req.indexOf("/cmd/drive/");
    int posComma = req.indexOf(",");
    int lenVal = req.length();
    String motAStr = req.substring(posVal+lenCmd1, posComma);
    String motBStr = req.substring(posComma+1, lenVal-lenCmd2);

    //String printVal = "motorA: " +  motAStr + ", motorB: " + motBStr;
    Serial.println(String("motorA: " +  motAStr + ", motorB: " + motBStr));
    int motA = motAStr.toInt();
    int motB = motBStr.toInt();
    manualDrive(motA, motB);
}

//*********************************************************************************************************
String getExecutionTimeForStartTime(unsigned long startTime) {
  unsigned long endT = micros();
  unsigned long delta = endT - startTime;
  return String(delta) + " us";
}

//*********************************************************************************************************
//*********************************************************************************************************
void loop() {
  // Check if a client has connected
  client = server.available();
  if (!client) {
    //no client state is most of the time
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  //handleINRequest();
  //************************************************
  String req = client.readStringUntil('\r');    // Read the first line of the request
  Serial.println(req);
  client.flush();

  //Serial.print("free RAM: ");
  //Serial.println(String(freeRam()));

  String resHeader = "{\"status\":\"ok\",";
  String resFotter = "\"}";
  String jsonResponse;
  
  // Match the request
  if (req.indexOf("/cmd/drive/") != -1){
    startT = micros();
    //----------------------------------------------
    prepareAndRunManualDrive(req);
    //----------------------------------------------
    Serial.println("manualDrive execution time: " + getExecutionTimeForStartTime(startT));
    jsonResponse = "{\"status\":\"ok\"}";
    
  }else if (req.indexOf("/cmd/forward") != -1){
    driveFroward();
  }else if (req.indexOf("/cmd/back") != -1){
    driveBack();
  }else if (req.indexOf("/cmd/rotateleft") != -1){
    rotateLeft();
  }else if (req.indexOf("/cmd/rotateright") != -1){
    rotateRight();
  }else if (req.indexOf("/cmd/stop") != -1){
    stopMotors();
  }else if (req.indexOf("/cmd/speed/") != -1){
    //req -> "GET /cmd/speed/248 HTTP/1.1"
    String val = req.substring(15,18);
    Serial.print("CMD/speed/");
    Serial.println(val);
    dSpeed = val.toInt();
    updateSpeed();
  }else {
    Serial.println("invalid request");
    client.stop();
    return;
  }
  
  client.flush();

  //jsonResponse: {"status":"ok","sensor":"HC-SR04","distance":"481"}

  // Prepare the response
  int jsonSize = jsonResponse.length();
  String httpResponse = "HTTP/1.1 200 OK\r\n";
  httpResponse += "Content-Type: application/json;\r\n";
  httpResponse += "Content-Length:" + String(jsonSize) + ";\r\n\r\n";  //NOTE - after header comes empty line
  httpResponse += jsonResponse + ";\r\n";
  client.print(httpResponse);   // Send the response to the client
  client.println();
  
  client.stop();

  //Serial.print(httpResponse);

  delay(100);
  Serial.println("Client disonnected");
  Serial.println("---------------------------------------");

  // The client will actually be disconnected when the function returns and 'client' object is detroyed
}
