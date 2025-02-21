char buffer[8] = "SyNK";

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Put both into the same line
  ((int*)buffer)[2] = analogRead(A0);
  ((int*)buffer)[3] = analogRead(A1);
  Serial.write(buffer, 8);
  delay(1);
}
