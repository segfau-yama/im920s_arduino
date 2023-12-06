#include <SoftwareSerial.h>

SoftwareSerial IM920sSerial(8, 9);

void setup() {
  IM920sSerial.begin(19200);
  Serial.begin(19200);
}

void loop() {
  if (Serial.available()) {
    String send_data = Serial.readString();
    IM920sSerial.println(send_data);
  }
  if (IM920sSerial.available()) {
    char recv_data = IM920sSerial.read();
    Serial.print(recv_data);
  }
}
