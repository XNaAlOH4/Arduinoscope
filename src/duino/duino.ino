#define PACKETS 5
#define MAX_CHANNELS 2

int data_bytes, channels;
char buffer[4 + PACKETS*MAX_CHANNELS*2] = "SyNK";
int *data, off = 0;
#define OUT_PIN 5

void setup() {
  Serial.begin(9600);
  pinMode(OUT_PIN, OUTPUT);
  data = (int*)(buffer+4);

  // Wait till Serial port is connected
  Serial.println("A-ready");
  while(!Serial);
  Serial.println("Serial done");

  // Waits for user input on the Serial port to begin
  while(!Serial.available());
  // Read in number of channels and number of outputs via Serial port

  Serial.read();
  channels = ;
  out_pins = ;

  while(!Serial.available());// Wait to start
}

void loop() {
  for(int i = 0; i < out_pins; i++) {
    analogWrite(OUT_PIN+i, analogRead(A)/4);
  }
  int val = analogRead(A2)/4;
  analogWrite(OUT_PIN, val);

  // We should wait a few microseconds b4 we read in data to prevent us from reading in non steady state values
  delay(1);// 1ms

  data[off++] = analogRead(A0);
  data[off++] = analogRead(A1);

  switch(channels) {
    case 6:
      data[off++] = analogRead(A5);
    case 5:
      data[off++] = analogRead(A4);
    case 4:
      data[off++] = analogRead(A3);
    case 3:
      data[off++] = analogRead(A2);
    case 2:
      data[off++] = analogRead(A1);
    case 1:
      data[off++] = analogRead(A0);
      break;
  }

  if(off == CHANNELS*PACKETS) {
    Serial.write(buffer, data_bytes);
    off = 0;
  }

  delay(10);
}
