
#define DECODE_NEC          // Includes Apple and Onkyo
#include <Arduino.h>
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library

void setup() {
    Serial.begin(115200);
    
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));

      int i = 0;

  while (i < 2) {
    Serial.println("IR_Mode");
  //  i++;
            analogWrite(5,0);
            analogWrite(6,0); 
 //   delay(5);

    if (IrReceiver.decode()) {


        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
           
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
        Serial.println();

  
        IrReceiver.resume(); // Enable receiving of the next value
 //forward
        if (IrReceiver.decodedIRData.command == 0x81) {
            analogWrite(5,100);
            digitalWrite(4, HIGH);
            analogWrite(6,100);
            digitalWrite(7, LOW); 
            delay(100);           
        } 
//blackward       
        if (IrReceiver.decodedIRData.command == 0x88) {
            analogWrite(5,100);
            digitalWrite(4, LOW);
            analogWrite(6,100);
            digitalWrite(7, HIGH); 
            delay(100);           
        }
//Life     
        if (IrReceiver.decodedIRData.command == 0x90) {
            analogWrite(5,100);
            digitalWrite(4, HIGH);
            analogWrite(6,0);
            digitalWrite(7, LOW); 
            delay(80);           
        }
//Right        
        if (IrReceiver.decodedIRData.command == 0x82) {
            analogWrite(5,0);
            digitalWrite(4, HIGH);
            analogWrite(6,100);
            digitalWrite(7, LOW); 
            delay(80);           
        }
//Go_auto_mode       
        else if (IrReceiver.decodedIRData.command == 0x84) {
            analogWrite(5,100);
            digitalWrite(4, HIGH);
            analogWrite(6,100);
            digitalWrite(7, LOW);
            delay(300);
            analogWrite(5,0);
            analogWrite(6,0);   
            i++;    
        }

    }
    
  }
}


void loop() {

    Serial.println("AUTO_Mode");
    delay(5);
    
    while ((analogRead(A2) < 980 && analogRead(A0) > 980 && analogRead(A6) < 980)) {
    //  forward();

            analogWrite(5,80);
            digitalWrite(4, HIGH);
            analogWrite(6,80);
            digitalWrite(7, LOW);
      delay(10);
    }
    while ((analogRead(A2) > 980 && analogRead(A0) < 980 && analogRead(A6) < 980)) {
   //   left();
            analogWrite(5,80);
            digitalWrite(4, HIGH);
            analogWrite(6,0);
            digitalWrite(7, LOW);
      delay(10);
    }
    while ((analogRead(A2) < 980 && analogRead(A0) < 980 && analogRead(A6) > 980)) {
  //    right();
            analogWrite(5,0);
            digitalWrite(4, HIGH);
            analogWrite(6,80);
            digitalWrite(7, LOW); 
      delay(10);
    }
    
}
