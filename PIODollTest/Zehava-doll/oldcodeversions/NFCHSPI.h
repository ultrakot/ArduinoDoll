#pragma once
#include <MFRC522.h>
#include <SPI.h>
#include <Arduino.h>

class NFCHSPI {
public:
    NFCHSPI(uint8_t ss1, uint8_t ss2, int rst, uint8_t sck, uint8_t miso, uint8_t mosi);
    void begin();
    void poll();
    bool poll(byte* foundUID, byte& foundSize, int& readerIndex);
private:
    uint8_t _ss1, _ss2, _sck, _miso, _mosi;
    int _rst;
    MFRC522 *_reader1, *_reader2;
};
