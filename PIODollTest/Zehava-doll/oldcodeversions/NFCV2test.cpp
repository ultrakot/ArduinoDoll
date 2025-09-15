#include "NFCV2test.h"

NFCV2test::NFCV2test(uint8_t ssPin, uint8_t sck, uint8_t miso, uint8_t mosi)
    : _ssPin(ssPin), _sck(sck), _miso(miso), _mosi(mosi), _spi(VSPI), _ss(nullptr), _driver(nullptr), _rfid(nullptr) {}

void NFCV2test::begin() {
    Serial.begin(115200);
    while (!Serial) { /* wait for USB */ }
    _spi.begin(_sck, _miso, _mosi, _ssPin);
    pinMode(_ssPin, OUTPUT);
    digitalWrite(_ssPin, HIGH);
    _ss = new MFRC522DriverPinSimple(_ssPin);
    _driver = new MFRC522DriverSPI(*_ss, _spi);
    _rfid = new MFRC522(*_driver);
    _rfid->PCD_Init();
    delay(50);
    uint8_t version = _driver->PCD_ReadRegister(MFRC522::PCD_Register::VersionReg);
    Serial.print(F("MFRC522 version: 0x"));
    Serial.println(version, HEX);
    if (version == 0x00) {
        Serial.println(F("❌ Communication failure – check wiring & power"));
    } else if (version == 0x91 || version == 0x92) {
        Serial.println(F("✅ MFRC522 detected"));
    }
}

void NFCV2test::poll() {
    if (!_rfid->PICC_IsNewCardPresent() || !_rfid->PICC_ReadCardSerial())
        return;
    Serial.print(F("UID:"));
    for (byte i = 0; i < _rfid->uid.size; i++) {
        Serial.print(' ');
        if (_rfid->uid.uidByte[i] < 0x10) Serial.print('0');
        Serial.print(_rfid->uid.uidByte[i], HEX);
    }
    Serial.println();
    _rfid->PICC_HaltA();
    _rfid->PCD_StopCrypto1();
    delay(500);
}
