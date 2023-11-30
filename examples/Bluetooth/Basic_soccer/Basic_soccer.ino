#define motor_speed 60

#include <SoftwareSerial.h>
#include <CAGEduino.h>

SoftwareSerial BT(11, A3);  // RX, TX

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  pinMode(MOTOR_L_PH, OUTPUT);
  pinMode(MOTOR_L_DIR, OUTPUT);
  pinMode(MOTOR_R_PH, OUTPUT);
  pinMode(MOTOR_R_DIR, OUTPUT);
  MOTOR_L(0);
  MOTOR_R(0);

  // set the data rate for the SoftwareSerial port
  BT.begin(9600);
}

void loop() {  // run over and over
  if (BT.available()) {
    char bt_get = BT.read();
    Serial.write(bt_get);
    if (bt_get == '0') {
      MOTOR_L(0);
      MOTOR_R(0);
    } else if (bt_get == 'w') {
      MOTOR_L(motor_speed);
      MOTOR_R(motor_speed);
    } else if (bt_get == 'a') {
      MOTOR_L(-motor_speed);
      MOTOR_R(motor_speed);
    } else if (bt_get == 's') {
      MOTOR_L(-motor_speed);
      MOTOR_R(-motor_speed);
    } else if (bt_get == 'd') {
      MOTOR_L(motor_speed);
      MOTOR_R(-motor_speed);
    } else if (bt_get == 'x') {
      MOTOR_L(motor_speed / 2);
      MOTOR_R(motor_speed);
    }

    else if (bt_get == 'b') {
      MOTOR_L(motor_speed);
      MOTOR_R(motor_speed / 2);
    }

    else if (bt_get == 'v') {
      MOTOR_L(-motor_speed / 2);
      MOTOR_R(-motor_speed);
    }

    else if (bt_get == 'c') {
      MOTOR_L(-motor_speed);
      MOTOR_R(-motor_speed / 2);
    }

    if (Serial.available()) {
      BT.write(Serial.read());
    }
  }
}
