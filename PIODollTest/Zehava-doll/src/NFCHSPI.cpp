#include "NFCHSPI.h"

NFCHSPI::NFCHSPI(uint8_t ss1, uint8_t ss2, int rst, uint8_t sck, uint8_t miso, uint8_t mosi)
    : _ss1(ss1), _ss2(ss2), _rst(rst), _sck(sck), _miso(miso), _mosi(mosi), _reader1(nullptr), _reader2(nullptr) {}

void NFCHSPI::begin() {
    Serial.begin(115200);
    while (!Serial);
    SPI.begin(_sck, _miso, _mosi, _ss1);
    pinMode(_ss1, OUTPUT);
    pinMode(_ss2, OUTPUT);
    digitalWrite(_ss1, HIGH);
    digitalWrite(_ss2, HIGH);
    _reader1 = new MFRC522(_ss1, _rst);
    _reader2 = new MFRC522(_ss2, _rst);
    _reader1->PCD_Init();
    delay(50);
    _reader2->PCD_Init();
    delay(50);
    byte ver1 = _reader1->PCD_ReadRegister(_reader1->VersionReg);
    Serial.print(F("Reader1 MFRC522 version: 0x"));
    Serial.println(ver1, HEX);
    if (ver1 == 0x00) {
        Serial.println(F("❌ Reader1 communication failure – check wiring & power"));
    }
    byte ver2 = _reader2->PCD_ReadRegister(_reader2->VersionReg);
    Serial.print(F("Reader2 MFRC522 version: 0x"));
    Serial.println(ver2, HEX);
    if (ver2 == 0x00) {
        Serial.println(F("❌ Reader2 communication failure – check wiring & power"));
    }
}

void NFCHSPI::poll() {
    // Reader 1
    if (_reader1->PICC_IsNewCardPresent() && _reader1->PICC_ReadCardSerial()) {
        Serial.print(F("Reader1 UID:"));
        for (byte i = 0; i < _reader1->uid.size; i++) {
            Serial.print(' ');
            Serial.print(_reader1->uid.uidByte[i], HEX);
        }
        Serial.println();
        _reader1->PICC_HaltA();
        _reader1->PCD_StopCrypto1();
        delay(200);
    }
    // Reader 2
    if (_reader2->PICC_IsNewCardPresent() && _reader2->PICC_ReadCardSerial()) {
        Serial.print(F("Reader2 UID:"));
        for (byte i = 0; i < _reader2->uid.size; i++) {
            Serial.print(' ');
            Serial.print(_reader2->uid.uidByte[i], HEX);
        }
        Serial.println();
        _reader2->PICC_HaltA();
        _reader2->PCD_StopCrypto1();
        delay(200);
    }
}
