#pragma once
#include <MFRC522.h>
#include <SPI.h>
#include <Arduino.h>

// Unified bus type enum
enum BusType { BUS_NONE, BUS_VSPI, BUS_HSPI };

struct ReaderConfig {
    uint8_t csPin;
    BusType bus;
    bool enabled;
};

static const int MAX_READERS = 5;

class NFCManagerV2 {
public:
    NFCManagerV2();
    void begin();
    void pollReaders();
    void printUid(const char* label, MFRC522& reader);
    void readNTAG(MFRC522& reader);
    ReaderConfig readerConfigs[MAX_READERS];
    MFRC522* readers[MAX_READERS];
};
