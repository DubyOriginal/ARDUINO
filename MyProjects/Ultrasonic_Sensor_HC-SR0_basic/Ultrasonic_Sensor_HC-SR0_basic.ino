 
    // Defines Tirg and Echo pins of the Ultrasonic Sensor
    const int echoPin = 7;
    const int trigPin = 8;

    long duration;
    int distance;
    
    void setup() {
      Serial.begin(9600);
      pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
      pinMode(echoPin, INPUT); // Sets the echoPin as an Input
    }
    
    void loop() {

      digitalWrite(trigPin, LOW); 
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH); 
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      
      duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
      distance= duration*0.34/2;

      Serial.print("s = ");
      Serial.print(distance); // Sends the distance value into the Serial Port
      Serial.println(" mm");
      
      Serial.print("t = ");
      Serial.print(duration); // Sends the distance value into the Serial Port
      Serial.println(" us");

      Serial.println("---------------------------------------");
      delay(1500);
      
    }

