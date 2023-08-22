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

void sing(int soun_num = 3) {
  int* melody;
  int* tempo;
  int size = 0;

  if(soun_num == 0){
    melody = melody1;
    tempo = duration1;
    size = melody_size[0];
  }
  else if(soun_num == 1){
    melody = melody2;
    tempo = duration2;
    size = melody_size[1];
  }
  else if(soun_num == 2){
    melody = melody3;
    tempo = duration3;
    size = melody_size[2];
  }
  else if(soun_num == 3){
    melody = melody4;
    tempo = duration4;
    size = melody_size[3];
  }

  for (int thisNote = 0; thisNote < size; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / tempo[thisNote];

    buzz(BUZZER_PIN , melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    // stop the tone playing:
    buzz(BUZZER_PIN , 0, noteDuration);

  }

}

void buzz(int targetPin, long frequency, long length) {
  digitalWrite(13, HIGH);
  long delayValue = 1000000 / frequency / 2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length / 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  for (long i = 0; i < numCycles; i++) { // for the calculated length of time...
    digitalWrite(targetPin, HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin, LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }
  digitalWrite(13, LOW);

}

void cageboard_init(){
  pinMode(USER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(USER_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN , OUTPUT);
}
