void setup() {
  pinMode(D0, INPUT);
  Serial.begin(9600);

}

void loop() {
  if (digitalRead(D0) == HIGH) {
    Serial.println("Sensor is touched");
    delay(500);
  }
  else {
    Serial.println("Sensor is not touched");
    delay(500);
  }
}
