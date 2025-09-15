#pragma once
#include <MFRC522.h>
#include <SPI.h>
#include <Arduino.h>

class NFCVSPI {
public:
    NFCVSPI(uint8_t ss1, uint8_t ss2, uint8_t ss3, int rst);
    void begin();
    void poll();
    // Polls all readers until the given UID is detected, returns true if found
    bool pollUntilUID(const byte *uid, byte uidSize);
    // Expose these for main.cpp access
    uint8_t _ss1, _ss2, _ss3;
    MFRC522 *_reader1, *_reader2, *_reader3;
private:
    void pollReader(MFRC522 &rfid, uint8_t ssPin, byte *lastUID, int which);
    void reportTag(MFRC522 &rfid, int which);
    int _rst;
    byte _lastUID1[4], _lastUID2[4], _lastUID3[4];
};
