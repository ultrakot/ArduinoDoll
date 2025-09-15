#pragma once
#include <MFRC522.h>
#include <SPI.h>
#include <Arduino.h>

class NFConeTest {
public:
    NFConeTest(uint8_t csPin, uint8_t sck, uint8_t miso, uint8_t mosi);
    void begin();
    void poll();
    void printUid();
    void readNTAG();
private:
    uint8_t _csPin, _sck, _miso, _mosi;
    MFRC522* _reader;
};
