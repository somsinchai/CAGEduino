#define USER_BUTTON_PIN 9
#define USER_LED_PIN 13

void setup() {
  pinMode(USER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(USER_LED_PIN, OUTPUT);
}

void loop() {
  if (digitalRead(USER_BUTTON_PIN) == LOW) { // Button pressed
    digitalWrite(USER_LED_PIN, HIGH);
  }
  else { // Button Release
    digitalWrite(USER_LED_PIN, HIGH);
    delay(400);
    digitalWrite(USER_LED_PIN, LOW);
    delay(400);
  }
}
