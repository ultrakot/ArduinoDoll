#include <Arduino.h>

#if !defined(ARDUINO_ESP32C3_DEV)
#define DISABLE_CODE_FOR_RECEIVER
#endif

#include <IRremote.hpp> // include the library

// === Define your send pin here ===
#define IR_SEND_PIN 17

uint8_t sCommand = 0x34;
uint8_t sRepeats = 0;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    Serial.println(F("START IR sender on pin 7"));

    // Explicitly initialize IrSender on pin 7
    IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
}

void loop() {
    Serial.println();
    Serial.print(F("Send now: address=0x00, command=0x"));
    Serial.print(sCommand, HEX);
    Serial.print(F(", repeats="));
    Serial.println(sRepeats);

    // Send NEC code on pin 7
    IrSender.sendNEC(0x00, sCommand, sRepeats);

    sCommand += 0x11;
    sRepeats++;
    if (sRepeats > 4) {
        sRepeats = 4;
    }

    delay(1000);
}
