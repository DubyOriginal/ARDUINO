#include <ESP8266WiFi.h>
#include <SHT1x.h>
/*
  D0   = 16;
  D1   = 5;
  D2   = 4;
  D3   = 0;
  D4   = 2;
  D5   = 14;
  D6   = 12;
  D7   = 13;
  D8   = 15;
  D9   = 3;
  D10  = 1;
*/

//CONST
//***********************************************
const char* ssid = "SkyNet";
const char* password = "adidasneo";

// Ultrasonic Sensor
//********************
int echoPin = 4;  //gpio 4  -> D2
int trigPin = 5;  //gpio 5  -> D1

// SHT11
//********************
#define dataPin  D5
#define clockPin 11
SHT1x sht1x(dataPin, clockPin);

//********************
int analogPin = A0;  //gpio 16  -> A0


//VAR
//***********************************************
long duration;
int distance;
unsigned long startT = 0;

// Create an instance of the server, specify the port to listen on as an argument
WiFiServer server(80);

//*********************************************************************************************************
//*********************************************************************************************************
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("");
  Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("MowerApp v1.1.5");

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

void preparePINS() {
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input

  pinMode(analogPin, INPUT);   // Sets the analogPin as an Input
}

//*********************************************************************************************************

//UltraSound sensor
//*********************************************************************************************************
String readHCSR04() {
  Serial.print("readHCSR04 -> ");

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration * 0.34 / 2;

  Serial.print("s = ");
  Serial.print(distance); // Sends the distance value into the Serial Port
  Serial.print(" mm");

  Serial.print(" (t = ");
  Serial.print(duration); // Sends the distance value into the Serial Port
  Serial.println(" us)");

  String disStr = String(distance);
  return "\"sensor\":\"HC-SR04\",\"distance\":\"" + disStr;

}

//potenciometer
//*********************************************************************************************************
String readAnalog() {
  Serial.print("readAnalog -> ");
  int analog = analogRead(analogPin);    // read the input pin
  Serial.print("AN = ");
  Serial.println(analog);

  String analogStr = String(analog);
  return "\"sensor\":\"analog\",\"analog\":\"" + analogStr;

}

//temperature and humidity sensor
//*********************************************************************************************************
String readSHT11() {
  Serial.print("readSHT11 -> ");
  
  float temp = sht1x.readTemperatureC();
  float humidity = sht1x.readHumidity();

  Serial.print("temp = ");
  Serial.print(temp);
  Serial.print("°C  /  humidity = ");
  Serial.print(humidity);
  Serial.println("%");

  String tempStr = String(temp);
  String humidityStr = String(humidity);
  return "\"sensor\":\"SHT11\",\"temp\":\"" + tempStr + "\",\"humidity\":\"" + humidity + "\"";

}

//*********************************************************************************************************
String ledON() {
  digitalWrite(trigPin, HIGH);
  Serial.println("---------------------------------------");
  return "\"sensor\":\"LED\", \"status\":\"ON\"";
}

//*********************************************************************************************************
String ledOFF() {
  digitalWrite(trigPin, LOW);
  Serial.println("---------------------------------------");
  return "\"sensor\":\"LED\", \"status\":\"OFF\"";
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
  WiFiClient client = server.available();
  if (!client) {
    //no client state is most of the time
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  //handleINRequest();
  //************************************************
  String req = client.readStringUntil('\r');    // Read the first line of the request
  Serial.println(req);
  client.flush();

  String resHeader = "{\"status\":\"ok\",";
  String resFotter = "\"}";
  String jsonResponse;

  // Match the request
  if (req.indexOf("/sensor/hcsr04") != -1) {
    startT = micros();
    //----------------------------------------------
    String distance = readHCSR04();
    //----------------------------------------------
    Serial.println("readHCSR04 execution time: " + getExecutionTimeForStartTime(startT));
    jsonResponse = resHeader + distance + resFotter;
    //Serial.println(jsonResponse);

  } else if (req.indexOf("/sensor/analog") != -1) {
    startT = micros();
    //----------------------------------------------
    String analog = readAnalog();
    //----------------------------------------------
    Serial.println("readAnalog execution time: " + getExecutionTimeForStartTime(startT));
    jsonResponse = resHeader + analog + resFotter;

  } else if (req.indexOf("/sensor/sht11") != -1) {
    startT = micros();
    //----------------------------------------------
    String sht11Val = readSHT11();
    //----------------------------------------------
    Serial.println("readSHT11 execution time: " + getExecutionTimeForStartTime(startT));
    jsonResponse = resHeader + sht11Val + resFotter;

  } else if (req.indexOf("/led/on") != -1) {
    String ledStatus = ledON();
    jsonResponse = resHeader + ledStatus + resFotter;
    Serial.println("/led/on");

  } else if (req.indexOf("/led/off") != -1) {
    String ledStatus = ledOFF();
    jsonResponse = resHeader + ledStatus + resFotter;
    Serial.println("/led/off");

  } else if (req.indexOf("/sensor/all") != -1) {
    Serial.println("/sensor/all");

  } else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  client.flush();   // clead out the input buffer:

  //jsonResponse: {"status":"ok","sensor":"HC-SR04","distance":"481"}

  // Prepare the response
  int jsonSize = jsonResponse.length();
  String httpResponse = "HTTP/1.1 200 OK\r\n";
  httpResponse += "Content-Type: application/json;\r\n";
  httpResponse += "Content-Length:" + String(jsonSize) + ";\r\n\r\n";  //NOTE - after header comes empty line
  httpResponse += jsonResponse + ";\r\n";
  client.print(httpResponse);   // Send the response to the client
  client.println();

  //Serial.print(httpResponse);

  delay(1);
  Serial.println("Client disonnected");
  Serial.println("---------------------------------------");

  // The client will actually be disconnected when the function returns and 'client' object is detroyed
}
