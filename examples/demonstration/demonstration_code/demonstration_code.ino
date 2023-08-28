#define IR_RECEIVE_PIN  2
#define melodyPin 10
#define BUT_PIN 9

#include <EEPROM.h>
#include <Arduino.h>
#include <CAGEduino.h>
#include <Wire.h>
#include <TM1650.h>
#include <Adafruit_NeoPixel.h>
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>


// 1->0x87 2->0x86 3->0x85 4->0x8b 5->0x8a 6->0x89 7->0x8f 8->0x8e 9->0x8d 0->0x92
// left->0x90 up->0x81 down->0X88 right->0x82 ok->0x99
// menu 0x93 exit->0x96
// A->0xc1 B->0xc2 C->0xc3 D->0xc0
// E->0xcc F->0xca G->0xcb E->0xc8
// youtube->0x91

volatile unsigned char remote_hash[255] = {0};

TM1650 d;
Adafruit_NeoPixel pixels(2, 8, NEO_GRB + NEO_KHZ800);


unsigned long IO_previousMillis = 0;
const long IO_interval = 2;

unsigned long APP_previousMillis = 0;
const long APP_interval = 10;

volatile int mode = 0;
volatile int mode_change = 0;

volatile unsigned long last_but_sec = 0;
volatile int but_state = 0;
volatile int but_hold_state = 0;
volatile int last_but_state = 0;

volatile int remote_last_command = -1;


volatile unsigned int track_sensor_upper_th[5];
volatile unsigned int track_sensor_lower_th[5];
volatile unsigned int track_sensor_range[5];

volatile unsigned int track_sensor_value;
volatile unsigned int track_sensor_raw_value[5];
volatile unsigned long track_sensor_norm_value[5];

int draw_flag = 0;
unsigned int prog_digi = 0;
byte prog_dir = 0;

const unsigned long weights[5] = {0, 250, 500, 750, 1000};
const unsigned long sum_weight = 2500;

unsigned long track_sensor_read() {
  track_sensor_raw_value[0] = (1024 - analogRead(A6)); track_sensor_raw_value[0] = (1024 - analogRead(A6));
  track_sensor_raw_value[1] = (1024 - analogRead(A7)); track_sensor_raw_value[1] = (1024 - analogRead(A7));
  track_sensor_raw_value[2] = (1024 - analogRead(A0)); track_sensor_raw_value[2] = (1024 - analogRead(A0));
  track_sensor_raw_value[3] = (1024 - analogRead(A1)); track_sensor_raw_value[3] = (1024 - analogRead(A1));
  track_sensor_raw_value[4] = (1024 - analogRead(A2)); track_sensor_raw_value[4] = (1024 - analogRead(A2));

  for (int i = 0; i < 5; i++) {
    if (track_sensor_raw_value[i] < track_sensor_lower_th[i]) {
      track_sensor_raw_value[i] = track_sensor_lower_th[i];
    }
    if (track_sensor_raw_value[i] > track_sensor_upper_th[i]) {
      track_sensor_raw_value[i] = track_sensor_upper_th[i];
    }

    track_sensor_norm_value[i] = (track_sensor_raw_value[i] - track_sensor_lower_th[i]);
    track_sensor_norm_value[i] *= 1000;
    track_sensor_norm_value[i] /= track_sensor_range[i];
  }

  for (int i = 0; i < 5; i++) {
    Serial.print(track_sensor_raw_value[i]);
    if (i < 4) {
      Serial.print(",");
    }
  }
  Serial.print("   ");
  for (int i = 0; i < 5; i++) {
    Serial.print(track_sensor_norm_value[i]);
    if (i < 4) {
      Serial.print(",");
    }
  }

  Serial.println(",");

  unsigned long ret = (track_sensor_norm_value[0] * weights[0]) + (track_sensor_norm_value[1] * weights[1]) + (track_sensor_norm_value[2] * weights[2]) + (track_sensor_norm_value[3] * weights[3]) + (track_sensor_norm_value[4] * weights[4]);
  unsigned long sum_x = (track_sensor_norm_value[0] + track_sensor_norm_value[1] + track_sensor_norm_value[2] + track_sensor_norm_value[3] + track_sensor_norm_value[4]);
  ret /= sum_x;
  Serial.println(ret);

  if (sum_x < 1000) {
    ret = 3000;
  }

  return ret;
}

void set_remote_hash() {

  for (int i = 0; i < 255; i++) {
    remote_hash[i] = 255;
  }
  remote_hash[0x87] = 1;
  remote_hash[0x86] = 2;
  remote_hash[0x85] = 3;
  remote_hash[0x8b] = 4;
  remote_hash[0x8a] = 5;
  remote_hash[0x89] = 6;
  remote_hash[0x8f] = 7;
  remote_hash[0x8e] = 8;
  remote_hash[0x8d] = 9;
  remote_hash[0x92] = 0;
  remote_hash[0xc1] = 10; //A
  remote_hash[0xc2] = 11; //B
  remote_hash[0xc3] = 12; //C
  remote_hash[0xc0] = 13; //D
  remote_hash[0xcc] = 14; //E
  remote_hash[0xca] = 15; //F
  remote_hash[0xcb] = 16; //G
  remote_hash[0xc8] = 17; //H
}


char cstr[4];

void setup() {
  Wire.begin();
  Serial.begin(115200);

  pinMode(3, OUTPUT);//buzzer
  pinMode(13, OUTPUT);//led indicator when singing a note

  pinMode(MOTOR_L_PH, OUTPUT);
  pinMode(MOTOR_L_DIR, OUTPUT);
  pinMode(MOTOR_R_PH, OUTPUT);
  pinMode(MOTOR_R_DIR, OUTPUT);
  MOTOR_L(0);
  MOTOR_R(0);

  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  set_remote_hash();

  pixels.begin();
  pixels.clear();

  pixels.setPixelColor(0, pixels.Color(0, 100, 0));
  pixels.setPixelColor(1, pixels.Color(0, 0, 100));
  pixels.show();

  d.init();
  d.displayOff();
  d.displayString("____");
  d.setBrightness(TM1650_MIN_BRIGHT);
  d.setBrightness(5);

  d.displayOn();
  d.displayString("CAGE");

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);


  printActiveIRProtocols(&Serial);
  Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));

  sing(0);
  d.displayString("Bot ");
  delay(1000);
  d.displayString("RDY ");
  pixels.setPixelColor(0, pixels.Color(5, 0, 0));
  pixels.setPixelColor(1, pixels.Color(0, 5, 0));
  pixels.show();
  pinMode(BUT_PIN, INPUT_PULLUP);

  // try to load sensor value

  for (int i = 0; i < 5; i++) { // load

    track_sensor_lower_th[i] = (EEPROM.read(5 + (i * 4) + 1) & 0x00FF);
    track_sensor_lower_th[i] <<= 8;
    track_sensor_lower_th[i] |= (EEPROM.read(5 + (i * 4) + 0) & 0x00FF);

    track_sensor_upper_th[i] = (EEPROM.read(5 + (i * 4) + 3) & 0x00FF);
    track_sensor_upper_th[i] <<= 8;
    track_sensor_upper_th[i] |= (EEPROM.read(5 + (i * 4) + 2) & 0x00FF);
  }

  for (int i = 0; i < 5; i++) {
    track_sensor_range[i] = track_sensor_upper_th[i] - track_sensor_lower_th[i];
  }



  last_but_sec = millis();
}

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - IO_previousMillis >= IO_interval) {
    IO_previousMillis = currentMillis;

    if (IrReceiver.decode()) {
      // 1->0x87 2->0x86 3->0x85 4->0x8b 5->0x8a 6->0x89 7->0x8f 8->0x8e 9->0x8d 0->0x92
      // left->0x90 up->0x81 down->0X88 right->0x82 ok->0x99
      // menu 0x93 exit->0x96
      // A->0xc1 B->0xc2 C->0xc3 D->0xc0
      // youtube->0x91

      if (IrReceiver.decodedIRData.command == 0x10) {
        // do something
      } else if (IrReceiver.decodedIRData.command == 0x11) {
        // do something else
      }
      else if (IrReceiver.decodedIRData.command == 0x96) {
        //remote_last_command = 0x96;
      }

      //if (remote_hash[IrReceiver.decodedIRData.command] > 0) {
      remote_last_command = IrReceiver.decodedIRData.command;
      //}

      IrReceiver.resume();
    }

    but_state = digitalRead(BUT_PIN);
    if (but_state != last_but_state && but_state == LOW) {
      last_but_sec = currentMillis;

    }
    if (but_state == LOW) {

      Serial.println(currentMillis - last_but_sec);
      if (currentMillis - last_but_sec > 3000) {
        but_hold_state = 1; // hold
      }
    }

    last_but_state = but_state;
  }


  if (currentMillis - APP_previousMillis >= APP_interval) {
    APP_previousMillis = currentMillis;

    if (mode == 0) { // idle
      if (mode_change == 0) {


        if (draw_flag == 1) {
          d.displayString("RDY ");
          pixels.setPixelColor(0, pixels.Color(5, 0, 0));
          pixels.setPixelColor(1, pixels.Color(0, 5, 0));
          pixels.show();
          buzz(melodyPin, 3000, 200);
          remote_last_command = -1;
          but_hold_state = 0;
          draw_flag = 0;
        }

        if (remote_last_command >= 0) {
          if ( remote_hash[remote_last_command] >= 10 && remote_hash[remote_last_command] <= 18 ) {
            mode_change = remote_hash[remote_last_command] - 9; // A to 1
            prog_digi = EEPROM.read(0x20 + (mode_change * 2));
            prog_dir = EEPROM.read(0x20 + (mode_change * 2) + 1);
            Serial.print("run ");
            Serial.println(mode_change);
            draw_flag = 1;
            remote_last_command = -1;
          }
        }
      }
      else if (mode_change >= 1) { // tracking
        if (draw_flag == 1) {
          Serial.println("running");
          char disp[] = "    ";
          disp[0] = 'A' + (mode_change - 1);
          if (prog_digi >= 100) {
            disp[1] = 'E';
            disp[2] = 'E';
          }
          else {
            if (prog_digi < 10) {
              disp[1] = ' ';
            }
            else {
              disp[1] = '0' + (prog_digi / 10);
            }
            disp[2] = '0' + (prog_digi % 10);
          }
          if (prog_dir == 0) { // left;
            disp[3] = '7';
          }
          else {
            disp[3] = 'r';
          }

          d.displayString(disp);
          pixels.setPixelColor(0, pixels.Color(0, 5, 5));
          pixels.setPixelColor(1, pixels.Color(5, 5, 0));
          pixels.show();
          //(melodyPin, 700 + (mode_change * 20), 100);
        }

        for (int i = 0; i < 1000; i++) {
          int sensor_read = track_sensor_read();
          int target = 500;
          int P = sensor_read - 500;
          int max_reduce_speed = prog_digi / 3;
          int speed_reduce = P * max_reduce_speed / 100;

          if (sensor_read <= 1000) {
            MOTOR_L(prog_digi - speed_reduce);
            MOTOR_R(prog_digi + speed_reduce);
          }
          else {
            if (prog_dir == 0) { // left
              MOTOR_L(prog_digi / 5);
              MOTOR_R(prog_digi);
            }
            else if (prog_dir == 1) {
              MOTOR_L(prog_digi);
              MOTOR_R(prog_digi / 5);
            }
            else {
              MOTOR_L(prog_digi / 2);
              MOTOR_R(prog_digi / 2);
            }
          }

          delay(1);
        }

        MOTOR_L(0);
        MOTOR_R(0);
        remote_last_command = -1;
        IrReceiver.resume();
        but_hold_state = 0;
        draw_flag = 1;
        mode_change = 0; // go back to idle

      }



      if (but_hold_state == 1) {
        mode_change = 0;
        mode = 1;
        but_hold_state = 0;
        draw_flag = 1;
      }

    }
    else if (mode == 1) { // main menu
      if (mode_change == 0) { // welcome
        if (draw_flag == 1) {
          Serial.println("haruhi2");
          d.displayString("PROG");
          pixels.setPixelColor(0, pixels.Color(5, 5, 0));
          pixels.setPixelColor(1, pixels.Color(5, 5, 0));
          pixels.show();
          buzz(melodyPin, 1000, 500);
          remote_last_command = -1;
          draw_flag = 0;
          but_hold_state = 0;
        }
        Serial.println(remote_last_command);

        if ( remote_hash[remote_last_command] >= 10 && remote_hash[remote_last_command] <= 18 ) {
          mode_change = remote_hash[remote_last_command] - 9; // A to 1
          prog_digi = EEPROM.read(0x20 + (mode_change * 2));
          prog_dir = EEPROM.read(0x20 + (mode_change * 2) + 1);
          Serial.print("enter ");
          Serial.println(mode_change);
          draw_flag = 1;
          remote_last_command = -1;
        }

        // test dir
        if (remote_last_command == 0x81) { // up
          d.displayString("UP  ");
          pixels.setPixelColor(0, pixels.Color(5, 5, 0));
          pixels.setPixelColor(1, pixels.Color(0, 5, 5));
          pixels.show();
          MOTOR_L(50);
          MOTOR_R(50);
          buzz(melodyPin, 1200, 4000);
          MOTOR_L(0);
          MOTOR_R(0);
          draw_flag = 1;
          remote_last_command = -1;
        }
        if (remote_last_command == 0x88) { // down
          d.displayString("DOWN");
          pixels.setPixelColor(0, pixels.Color(5, 5, 0));
          pixels.setPixelColor(1, pixels.Color(0, 5, 5));
          pixels.show();
          MOTOR_L(-50);
          MOTOR_R(-50);
          buzz(melodyPin, 1200, 4000);
          MOTOR_L(0);
          MOTOR_R(0);
          draw_flag = 1;
          remote_last_command = -1;
        }
        if (remote_last_command == 0x90) { // left
          d.displayString("LEFT");
          pixels.setPixelColor(0, pixels.Color(5, 5, 0));
          pixels.setPixelColor(1, pixels.Color(0, 5, 5));
          pixels.show();
          MOTOR_L(-50);
          MOTOR_R(50);
          buzz(melodyPin, 1200, 4000);
          MOTOR_L(0);
          MOTOR_R(0);
          draw_flag = 1;
          remote_last_command = -1;
        }
        if (remote_last_command == 0x82) { // right
          d.displayString("RIGH");
          pixels.setPixelColor(0, pixels.Color(5, 5, 0));
          pixels.setPixelColor(1, pixels.Color(0, 5, 5));
          pixels.show();
          MOTOR_L(50);
          MOTOR_R(-50);
          buzz(melodyPin, 1200, 4000);
          MOTOR_L(0);
          MOTOR_R(0);
          draw_flag = 1;
          remote_last_command = -1;
        }
        // left->0x90 up->0x81 down->0X88 right->0x82 ok->0x99
        // 1->0x87 2->0x86
        //test sensor
        if (remote_last_command == 0x87) { // 1
          pixels.setPixelColor(0, pixels.Color(5, 5, 0));
          pixels.setPixelColor(1, pixels.Color(0, 5, 5));
          pixels.show();


          for (int i = 0; i < 400; i++) {
            unsigned int sensor_read = track_sensor_read();

            cstr[3] = '0' + sensor_read % 10;
            cstr[2] = '0' + (sensor_read / 10) % 10;
            cstr[1] = '0' + (sensor_read / 100) % 10;
            cstr[0] = '0';
            d.displayString(cstr);
            delay(50);
          }


          draw_flag = 1;
          remote_last_command = -1;
        }

        // cal sensor
        if (remote_last_command == 0x86) { // 2
          d.displayString("CAL ");

          pixels.setPixelColor(0, pixels.Color(0, 0, 0));
          pixels.setPixelColor(1, pixels.Color(0, 0, 0));
          pixels.show();

          //volatile unsigned int track_sensor_upper_th[5];
          //volatile unsigned int track_sensor_lower_th[5];
          //volatile unsigned int track_sensor_range[5];
          //  track_sensor_raw_value[0] = analogRead(A6);
          //  track_sensor_raw_value[1] = analogRead(A7);
          //  track_sensor_raw_value[2] = analogRead(A0);
          //  track_sensor_raw_value[3] = analogRead(A1);
          //  track_sensor_raw_value[4] = analogRead(A2);

          unsigned int sensor_read = 0;
          for (int i = 0; i < 5; i++) {
            track_sensor_upper_th[i] = 0;
            track_sensor_lower_th[i] = 1024;
          }

          for (int i = 400; i >= 0; i--) {
            sensor_read = (1024 - analogRead(A6)); sensor_read = (1024 - analogRead(A6));
            if (sensor_read > track_sensor_upper_th[0]) track_sensor_upper_th[0] = sensor_read;
            if (sensor_read < track_sensor_lower_th[0]) track_sensor_lower_th[0] = sensor_read;
            delay(5);

            sensor_read = (1024 - analogRead(A7)); sensor_read = (1024 - analogRead(A7));
            if (sensor_read > track_sensor_upper_th[1]) track_sensor_upper_th[1] = sensor_read;
            if (sensor_read < track_sensor_lower_th[1]) track_sensor_lower_th[1] = sensor_read;
            delay(5);

            sensor_read = (1024 - analogRead(A0)); sensor_read = (1024 - analogRead(A0));
            if (sensor_read > track_sensor_upper_th[2]) track_sensor_upper_th[2] = sensor_read;
            if (sensor_read < track_sensor_lower_th[2]) track_sensor_lower_th[2] = sensor_read;
            delay(5);

            sensor_read = (1024 - analogRead(A1)); sensor_read = (1024 - analogRead(A1));
            if (sensor_read > track_sensor_upper_th[3]) track_sensor_upper_th[3] = sensor_read;
            if (sensor_read < track_sensor_lower_th[3]) track_sensor_lower_th[3] = sensor_read;
            delay(5);

            sensor_read = (1024 - analogRead(A2)); sensor_read = (1024 - analogRead(A2));
            if (sensor_read > track_sensor_upper_th[4]) track_sensor_upper_th[4] = sensor_read;
            if (sensor_read < track_sensor_lower_th[4]) track_sensor_lower_th[4] = sensor_read;
            delay(5);

            cstr[3] = '0' + i % 10;
            cstr[2] = '0' + (i / 10) % 10;
            cstr[1] = '0' + (i / 100) % 10;
            cstr[0] = '0';
            d.displayString(cstr);
          }

          for (int i = 0; i < 5; i++) {
            track_sensor_range[i] = track_sensor_upper_th[i] - track_sensor_lower_th[i];
          }

          Serial.print("MAX");
          for (int i = 0; i < 5; i++) {
            Serial.print(track_sensor_upper_th[i]);
            if (i < 4) {
              Serial.print(",");
            }
          }
          Serial.println();
          Serial.print("MIN");
          for (int i = 0; i < 5; i++) {
            Serial.print(track_sensor_lower_th[i]);
            if (i < 4) {
              Serial.print(",");
            }
          }
          Serial.print("RANGE");
          for (int i = 0; i < 5; i++) {
            Serial.print(track_sensor_range[i]);
            if (i < 4) {
              Serial.print(",");
            }
          }
          Serial.println();

          for (int i = 0; i < 5; i++) { // save
            EEPROM.write(5 + (i * 4) + 0, track_sensor_lower_th[i] & 0x00FF);
            EEPROM.write(5 + (i * 4) + 1, (track_sensor_lower_th[i] >> 8) & 0x00FF);
            EEPROM.write(5 + (i * 4) + 2, track_sensor_upper_th[i] & 0x00FF);
            EEPROM.write(5 + (i * 4) + 3, (track_sensor_upper_th[i] >> 8) & 0x00FF);
          }

          draw_flag = 1;
          remote_last_command = -1;
        }

        if (remote_last_command == 0x85) { // 3
          d.displayString("3");
          pixels.setPixelColor(0, pixels.Color(5, 5, 0));
          pixels.setPixelColor(1, pixels.Color(0, 5, 5));
          pixels.show();



          for (int i = 0; i < 5; i++) {
            track_sensor_range[i] = track_sensor_upper_th[i] - track_sensor_lower_th[i];
          }

          Serial.print("MAX");
          for (int i = 0; i < 5; i++) {
            Serial.print(track_sensor_upper_th[i]);
            if (i < 4) {
              Serial.print(",");
            }
          }
          Serial.println();
          Serial.print("MIN");
          for (int i = 0; i < 5; i++) {
            Serial.print(track_sensor_lower_th[i]);
            if (i < 4) {
              Serial.print(",");
            }
          }
          Serial.print("RANGE");
          for (int i = 0; i < 5; i++) {
            Serial.print(track_sensor_range[i]);
            if (i < 4) {
              Serial.print(",");
            }
          }
          Serial.println();


          draw_flag = 1;
          remote_last_command = -1;
        }



      }
      else if (mode_change >= 1) { // welcome
        if (draw_flag == 1) {
          Serial.println("tune");
          char disp[] = "    ";
          disp[0] = 'A' + (mode_change - 1);
          if (prog_digi >= 100) {
            disp[1] = 'E';
            disp[2] = 'E';
          }
          else {
            if (prog_digi < 10) {
              disp[1] = ' ';
            }
            else {
              disp[1] = '0' + (prog_digi / 10);
            }
            disp[2] = '0' + (prog_digi % 10);
          }
          if (prog_dir == 0) { // left;
            disp[3] = '7';
          }
          else if (prog_dir == 1) { // right
            disp[3] = 'r';
          }
          else { // str
            disp[3] = 'l';
          }

          d.displayString(disp);
          pixels.setPixelColor(0, pixels.Color(0, 5, 5));
          pixels.setPixelColor(1, pixels.Color(5, 5, 0));
          pixels.show();
          buzz(melodyPin, 700 + (mode_change * 20), 100);

          remote_last_command = -1;
          IrReceiver.resume();
          but_hold_state = 0;
          draw_flag = 0;
        }

        Serial.println(remote_last_command);

        if (remote_last_command >= 0 ) {
          if ( remote_hash[remote_last_command] >= 10 && remote_hash[remote_last_command] <= 18 ) {
            mode_change = remote_hash[remote_last_command] - 9; // A to 1
            prog_digi = EEPROM.read(0x20 + (mode_change * 2));
            prog_dir = EEPROM.read(0x20 + (mode_change * 2) + 1);
            Serial.print("enter ");
            Serial.println(prog_digi);
            Serial.println(prog_dir);
            Serial.println(mode_change);
            draw_flag = 1;
            remote_last_command = -1;
          }

          if ( remote_hash[remote_last_command] >= 0 && remote_hash[remote_last_command] <= 9) { // digi
            if (prog_digi >= 100) {
              prog_digi = 0;
            }
            prog_digi *= 10;
            prog_digi += remote_hash[remote_last_command];
            prog_digi %= 100;
            draw_flag = 1;
            remote_last_command = -1;
            Serial.print("digi");
            Serial.println(prog_digi);
          }
          if (remote_last_command == 0x90) {
            prog_dir = 0;
            draw_flag = 1;
            remote_last_command = -1;
          }
          if (remote_last_command == 0x82) {
            prog_dir = 1;
            draw_flag = 1;
            remote_last_command = -1;
          }
          if (remote_last_command == 0x81) { // up
            prog_dir = 2;
            draw_flag = 1;
            remote_last_command = -1;
          }
          if (remote_last_command == 0x99) { // ok

            EEPROM.write(0x20 + (mode_change * 2), (prog_digi & 0xFF));
            EEPROM.write(0x20 + (mode_change * 2) + 1, prog_dir);
            mode_change = 0; // A to 1
            Serial.print("Save");
            Serial.println(prog_digi);
            Serial.println(prog_dir);
            Serial.println(mode_change);
            buzz(melodyPin, 1500, 100);
            buzz(melodyPin, 2000, 150);
            draw_flag = 1;
            remote_last_command = -1;
          }
        }
      }

      if (remote_last_command == 0x96) {
        mode = 0;
        mode_change = 0;
        draw_flag = 1;
        remote_last_command = -1;
      }

    }
    else if (mode == 2) {

    }

  }

}

// left->0x90 up->0x81 down->0X88 right->0x82 ok->0x99
