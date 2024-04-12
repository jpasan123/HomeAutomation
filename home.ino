#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include "DHT.h"
#include <Servo.h>


#define FIREBASE_HOST "smart-home-bafc2-default-rtdb.firebaseio.com" // Firebase host"
#define FIREBASE_AUTH "IHYN6YwO2LnSkCTqDydm4PqljfalPHigElh7zzXh" // Firebase Auth code
#define WIFI_SSID "RedmiNote10S" // Enter your WiFi Name
#define WIFI_PASSWORD "12345678" // Enter your WiFi password

int irPin = D7;   // IR sensor connected to pin D7
int servoPin = D0; // Servo motor connected to pin D0


Servo servo;

int fireStatus1 = 0;
int fireStatus2 = 0;

#define DHTPIN D5
#define DHTTYPE DHT11
#define FAN_PIN D8 // Replace D8 with the pin number connected to the fan
#define BUZZER_PIN D1 // Replace D1 with the pin number connected to the buzzer
DHT dht(DHTPIN, DHTTYPE);   
const int gasSensorPin = 6;
const int moistureSensorPin = A0; // Pin connected to the moisture sensor

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  dht.begin();

  pinMode(D2, OUTPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D4, OUTPUT);
  pinMode(irPin, INPUT);
  pinMode(moistureSensorPin, INPUT);
  servo.attach(servoPin);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.println("Connected.");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  float h = dht.readHumidity();
  Serial.println(h);
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  int gas = digitalRead(gasSensorPin);
  int moistureValue = analogRead(moistureSensorPin);

 // Check if temperature exceeds 31.9 degrees Celsius
  if (Firebase.getInt("SmartHome/Fan") == 1) {
    // Turn on the fan
    digitalWrite(FAN_PIN, HIGH);
    Serial.println("fn 1");
  } else {
    // Turn off the fan
    digitalWrite(FAN_PIN, LOW);
    Serial.println("fn 0");
  }

  if (gas == HIGH) {
    Firebase.setString("SmartHome/Gas","Gas leak detected"); // Gas leak detected
    Serial.println("Gas Leak detected");
    digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer

  } else {
    Firebase.setString("SmartHome/Gas","No Gas leak detected");  // No gas leak detected 
    Serial.println("No Gas Leak detected");
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
  }
  Firebase.setFloat("SmartHome/Temp",t);
  Firebase.setFloat("SmartHome/Humidity",h);
  // Use .getInt with the correct path to your Firebase data.
  fireStatus1 = Firebase.getInt("SmartHome/Light1");
  fireStatus2 = Firebase.getInt("SmartHome/Light2");
  if (fireStatus1 == 1) {
    Serial.println("LED Turned ON");
    digitalWrite(D2, HIGH);
  } else if (fireStatus1 == 0) {
    Serial.println("LED Turned OFF");
    digitalWrite(D2, LOW);
  } else {
    Serial.println("Command Error! Please send 0/1");
  }

  if (fireStatus2 == 1) {
    Serial.println("LED Turned ON");
    digitalWrite(D4, LOW);
  } else if (fireStatus2 == 0) {
    Serial.println("LED Turned OFF");
    digitalWrite(D4, HIGH);
  } else {
    Serial.println("Command Error! Please send 0/1");
  }

  int irValue = digitalRead(irPin);

  if (irValue == HIGH || Firebase.getString("SmartHome/Door") == "open") {
    // IR sensor detects presence, rotate servo to 0 degrees
    Serial.println("door open IR turned HIGH or firebase turned high");
    rotateServo(90);
  } else {
    // No presence detected, rotate servo to 90 degrees
    rotateServo(0);
  }
// Assuming the moisture sensor gives higher values for drier soil and lower values for wetter soil
  Firebase.setInt("SmartHome/Moisture", moistureValue);
  Serial.println("Moisture Detected");

  delay(500);
}

void rotateServo(int angle) {
  servo.write(angle);
  delay(200); // Adjust the delay as needed
}