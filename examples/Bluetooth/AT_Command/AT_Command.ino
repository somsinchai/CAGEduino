#define BT_NAME "CAGEBOTxxx"

#include <SoftwareSerial.h>

SoftwareSerial BT(11, A3); // RX, TX

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("Started AT Command Comunication");

  // set the data rate for the SoftwareSerial port
  BT.begin(9600);

  BT.print("AT+NAME");
  BT.println(BT_NAME);
}

void loop() { // run over and over
  if (BT.available()) {
    Serial.write(BT.read());
  }
  if (Serial.available()) {
    BT.write(Serial.read());
  }
}
