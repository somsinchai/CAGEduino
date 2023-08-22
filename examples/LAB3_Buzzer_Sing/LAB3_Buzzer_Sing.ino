#define USER_BUTTON_PIN 9
#define USER_LED_PIN 13
#define BUZZER_PIN 10

#include <CAGEduino.h>

const int *melody[10];

#include <Wire.h>
#include <TM1650.h>

TM1650 d;

void setup() {
  Wire.begin();
  
  cageboard_init();
  pinMode(USER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(USER_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN , OUTPUT);

  d.init();
  d.displayString("CAGE");
  delay(500);
  d.displayString("bot ");

}

void loop() {
  if (digitalRead(USER_BUTTON_PIN) == LOW) { // Button pressed
    sing(3);
  }
  else { // Button Release

  }
}
