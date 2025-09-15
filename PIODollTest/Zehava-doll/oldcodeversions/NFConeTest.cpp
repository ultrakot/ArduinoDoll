#include "NFConeTest.h"

NFConeTest::NFConeTest(uint8_t csPin, uint8_t sck, uint8_t miso, uint8_t mosi)
    : _csPin(csPin), _sck(sck), _miso(miso), _mosi(mosi), _reader(nullptr) {}

void NFConeTest::begin() {
    Serial.print("NFConeTest: Initializing reader on CS pin "); Serial.println(_csPin);
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    SPI.end();
    SPI.begin(_sck, _miso, _mosi, _csPin);
    _reader = new MFRC522(_csPin, -1);
    _reader->PCD_Init();
    delay(50);
    Serial.println("NFConeTest: Reader initialized");
}

void NFConeTest::poll() {
    Serial.println("NFConeTest: Polling for card...");
    digitalWrite(_csPin, LOW);
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    bool ok = _reader->PICC_IsNewCardPresent() && _reader->PICC_ReadCardSerial();
    Serial.print("Card present/read result: "); Serial.println(ok);
    if (ok) {
        printUid();
        readNTAG();
        _reader->PICC_HaltA();
        _reader->PCD_StopCrypto1();
        Serial.println("Card halted and crypto stopped");
    }
    SPI.endTransaction();
    digitalWrite(_csPin, HIGH);
    delay(100);
}

void NFConeTest::printUid() {
    Serial.print("NFConeTest: UID: ");
    for (byte i = 0; i < _reader->uid.size; i++) {
        Serial.print(_reader->uid.uidByte[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void NFConeTest::readNTAG() {
    Serial.println("NFConeTest: readNTAG called");
    // Read first 4 pages (16 bytes)
    byte buffer[18];
    byte size = sizeof(buffer);
    for (byte page = 0; page < 4; page++) {
        byte command[] = {0x30, page}; // 0x30 = READ, page address
        if (_reader->PCD_TransceiveData(command, sizeof(command), buffer, &size) == MFRC522::STATUS_OK) {
            Serial.print("Page "); Serial.print(page); Serial.print(": ");
            for (byte j = 0; j < 4; j++) {
                Serial.print(buffer[j], HEX); Serial.print(" ");
            }
            Serial.println();
        } else {
            Serial.print("Failed to read page "); Serial.println(page);
        }
    }
    Serial.println("NFConeTest: readNTAG complete");
}
