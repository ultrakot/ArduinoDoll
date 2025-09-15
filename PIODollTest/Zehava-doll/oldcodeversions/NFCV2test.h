#pragma once
#include <SPI.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <Arduino.h>

class NFCV2test {
public:
    NFCV2test(uint8_t ssPin, uint8_t sck, uint8_t miso, uint8_t mosi);
    void begin();
    void poll();
private:
    uint8_t _ssPin, _sck, _miso, _mosi;
    SPIClass _spi;
    MFRC522DriverPinSimple *_ss;
    MFRC522DriverSPI *_driver;
    MFRC522 *_rfid;
};
