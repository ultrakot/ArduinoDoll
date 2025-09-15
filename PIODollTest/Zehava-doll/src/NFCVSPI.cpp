#include "NFCVSPI.h"
#include <string.h>

NFCVSPI::NFCVSPI(uint8_t ss1, uint8_t ss2, uint8_t ss3, int rst)
    : _ss1(ss1), _ss2(ss2), _ss3(ss3), _rst(rst) {
    memset(_lastUID1, 0, sizeof(_lastUID1));
    memset(_lastUID2, 0, sizeof(_lastUID2));
    memset(_lastUID3, 0, sizeof(_lastUID3));
    _reader1 = nullptr;
    _reader2 = nullptr;
    _reader3 = nullptr;
}

void NFCVSPI::begin() {
    Serial.begin(115200);
    while (!Serial);
    SPI.begin(); // VSPI default: SCK=18, MISO=19, MOSI=23
    pinMode(_ss1, OUTPUT); digitalWrite(_ss1, HIGH);
    pinMode(_ss2, OUTPUT); digitalWrite(_ss2, HIGH);
    pinMode(_ss3, OUTPUT); digitalWrite(_ss3, HIGH);
    _reader1 = new MFRC522(_ss1, _rst);
    _reader2 = new MFRC522(_ss2, _rst);
    _reader3 = new MFRC522(_ss3, _rst);
    _reader1->PCD_Init();
    _reader2->PCD_Init();
    _reader3->PCD_Init();
    Serial.println("Ready: three-reader setup on default SPI");
}

void NFCVSPI::poll() {
    pollReader(*_reader1, _ss1, _lastUID1, 1);
    pollReader(*_reader2, _ss2, _lastUID2, 2);
    pollReader(*_reader3, _ss3, _lastUID3, 3);
    delay(50);
}

void NFCVSPI::pollReader(MFRC522 &rfid, uint8_t ssPin, byte *lastUID, int which) {
    digitalWrite(_ss1, HIGH);
    digitalWrite(_ss2, HIGH);
    digitalWrite(_ss3, HIGH);
    digitalWrite(ssPin, LOW);
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        // Always print UID, even if duplicate
        reportTag(rfid, which);
        // Optionally update lastUID for other logic
        memcpy(lastUID, rfid.uid.uidByte, 4);
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }
    digitalWrite(ssPin, HIGH);
}

void NFCVSPI::reportTag(MFRC522 &rfid, int which) {
    auto type = rfid.PICC_GetType(rfid.uid.sak);
    Serial.printf("Reader %d tag: %s\n", which, rfid.PICC_GetTypeName(type));
    Serial.print("  UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10) Serial.print('0');
        Serial.print(rfid.uid.uidByte[i], HEX);
        if (i < rfid.uid.size - 1) Serial.print(':');
    }
    Serial.println();
}

// Polls all readers until the given UID is detected, returns true if found
bool NFCVSPI::pollUntilUID(const byte *uid, byte uidSize) {
    MFRC522* readers[3] = {_reader1, _reader2, _reader3};
    uint8_t ssPins[3] = {_ss1, _ss2, _ss3};
    for (int i = 0; i < 3; i++) {
        if (readers[i]) {
            digitalWrite(_ss1, HIGH);
            digitalWrite(_ss2, HIGH);
            digitalWrite(_ss3, HIGH);
            digitalWrite(ssPins[i], LOW);
            if (readers[i]->PICC_IsNewCardPresent() && readers[i]->PICC_ReadCardSerial()) {
                // Compare UID
                if (readers[i]->uid.size == uidSize) {
                    bool match = true;
                    for (byte j = 0; j < uidSize; j++) {
                        if (readers[i]->uid.uidByte[j] != uid[j]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        readers[i]->PICC_HaltA();
                        readers[i]->PCD_StopCrypto1();
                        digitalWrite(ssPins[i], HIGH);
                        return true;
                    }
                }
                readers[i]->PICC_HaltA();
                readers[i]->PCD_StopCrypto1();
                delay(500);
            }
            digitalWrite(ssPins[i], HIGH);
        }
    }
    delay(50);
    return false;
}