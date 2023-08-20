#include<CAGEduino.h>

void MOTOR_L(int speed) {

  if (speed >= 0) {
    if (speed > 255) speed = 255;

    digitalWrite(MOTOR_L_DIR, HIGH);
    analogWrite(MOTOR_L_PH, speed);
  }
  else {
    if (speed < -255) speed = -255;

    digitalWrite(MOTOR_L_DIR, LOW);
    analogWrite(MOTOR_L_PH, -speed);
  }

}

void MOTOR_R(int speed) {

  if (speed >= 0) {
    if (speed > 255) speed = 255;

    digitalWrite(MOTOR_R_DIR, HIGH);
    analogWrite(MOTOR_R_PH, speed);
  }
  else {
    if (speed < -255) speed = -255;

    digitalWrite(MOTOR_R_DIR, LOW);
    analogWrite(MOTOR_R_PH, -speed);
  }

}