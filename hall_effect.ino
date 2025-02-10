#include <Arduino.h>

const int ledPin = 2;  // Built-in LED pin

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  Serial.println("ESP32 Hall Effect Sensor Test");
}

void loop() {
  int hallValue = hallRead();  // Alternative Hall sensor read function
  Serial.printf("Hall sensor measurement: %d\n", hallValue);

  // LED ON if strong magnetic field detected
  digitalWrite(ledPin, (abs(hallValue) > 50) ? HIGH : LOW);

  delay(1000);
}
