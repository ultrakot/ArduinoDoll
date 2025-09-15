#include "NFCManagerV2.h"

// Reads NTAG UID and first 4 pages (16 bytes)
void NFCManagerV2::readNTAG(MFRC522 &reader) {
    Serial.println("readNTAG called");
    if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
        Serial.print("NTAG UID: ");
        for (byte i = 0; i < reader.uid.size; i++) {
            Serial.print(reader.uid.uidByte[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        // Read first 4 pages (16 bytes)
        byte buffer[18];
        byte size = sizeof(buffer);
        for (byte page = 0; page < 4; page++) {
            byte command[] = {0x30, page}; // 0x30 = READ, page address
            if (reader.PCD_TransceiveData(command, sizeof(command), buffer, &size) == MFRC522::STATUS_OK) {
                Serial.print("Page "); Serial.print(page); Serial.print(": ");
                for (byte j = 0; j < 4; j++) {
                    Serial.print(buffer[j], HEX); Serial.print(" ");
                }
                Serial.println();
            } else {
                Serial.print("Failed to read page "); Serial.println(page);
            }
        }
        reader.PICC_HaltA();
        reader.PCD_StopCrypto1();
    }
    Serial.println("readNTAG complete");
}

NFCManagerV2::NFCManagerV2() {
    Serial.println("NFCManagerV2 constructor: initializing reader configs");
    readerConfigs[0] = {15, BUS_VSPI, false}; // VSPI reader 1
    readerConfigs[1] = {4,  BUS_VSPI, false}; // VSPI reader 2
    readerConfigs[2] = {5,  BUS_VSPI, false}; // VSPI reader 3
    readerConfigs[3] = {25, BUS_HSPI, false}; // HSPI reader 1
    readerConfigs[4] = {27, BUS_HSPI, false}; // HSPI reader 2
    for (int i = 0; i < MAX_READERS; ++i) readers[i] = nullptr;
    Serial.println("NFCManagerV2 constructor: configs initialized");
}

void NFCManagerV2::begin() {
    Serial.println("NFCManagerV2::begin() called");
    // Example: enable only VSPI readers
    readerConfigs[0].enabled = true;
    readerConfigs[1].enabled = false;
    readerConfigs[2].enabled = false;
    readerConfigs[3].enabled = false;
    readerConfigs[4].enabled = false;

    for (int i = 0; i < MAX_READERS; ++i) {
        Serial.print("Configuring reader "); Serial.println(i);
        if (readerConfigs[i].enabled) {
            Serial.print("Enabling reader on CS pin "); Serial.println(readerConfigs[i].csPin);
            pinMode(readerConfigs[i].csPin, OUTPUT);
            digitalWrite(readerConfigs[i].csPin, HIGH);
            Serial.print("Creating MFRC522 object for CS pin "); Serial.println(readerConfigs[i].csPin);
            readers[i] = new MFRC522(readerConfigs[i].csPin, -1);
            // Switch SPI bus if needed
            if (readerConfigs[i].bus == BUS_VSPI) {
                Serial.println("Switching SPI to VSPI for initialization");
                SPI.end();
                SPI.begin(18, 19, 23, readerConfigs[i].csPin);
            } else if (readerConfigs[i].bus == BUS_HSPI) {
                Serial.println("Switching SPI to HSPI for initialization");
                SPI.end();
                SPI.begin(14, 12, 13, readerConfigs[i].csPin);
            }
            Serial.print("Calling PCD_Init() for reader "); Serial.println(i);
            readers[i]->PCD_Init();
            delay(50);
            Serial.print("Reader "); Serial.print(i); Serial.println(" initialized");
        } else {
            Serial.print("Reader "); Serial.print(i); Serial.println(" is disabled");
        }
    }
    Serial.println("NFCManagerV2::begin() complete");
}

void NFCManagerV2::pollReaders() {
    Serial.println("NFCManagerV2::pollReaders() called");
    BusType lastBus = BUS_NONE;
    for (int i = 0; i < MAX_READERS; ++i) {
        Serial.print("Polling reader "); Serial.println(i);
        if (readerConfigs[i].enabled && readers[i]) {
            Serial.print("Reader "); Serial.print(i); Serial.println(" is enabled");
            // Switch SPI bus only if needed
            if (readerConfigs[i].bus != lastBus) {
                Serial.print("Switching SPI bus for reader "); Serial.println(i);
                SPI.end();
                delay(10);
                if (readerConfigs[i].bus == BUS_VSPI) {
                    Serial.println("Switching to VSPI");
                    SPI.begin(18, 19, 23, readerConfigs[i].csPin);
                } else if (readerConfigs[i].bus == BUS_HSPI) {
                    Serial.println("Switching to HSPI");
                    SPI.begin(14, 12, 13, readerConfigs[i].csPin);
                }
                delay(50);
                lastBus = readerConfigs[i].bus;
            }
            // Deselect all CS pins for this bus
            Serial.print("Deselecting all CS pins for bus "); Serial.println(lastBus);
            for (int j = 0; j < MAX_READERS; ++j) {
                if (readerConfigs[j].bus == lastBus && readerConfigs[j].enabled) {
                    digitalWrite(readerConfigs[j].csPin, HIGH);
                }
            }
            Serial.print("Selecting CS pin "); Serial.println(readerConfigs[i].csPin);
            digitalWrite(readerConfigs[i].csPin, LOW);
            SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
            Serial.println("Checking for new card...");
            bool ok = readers[i]->PICC_IsNewCardPresent() && readers[i]->PICC_ReadCardSerial();
            Serial.print("Card present/read result: "); Serial.println(ok);
            if (ok) {
                Serial.println("Card detected, printing UID and reading NTAG...");
                printUid("Reader UID:", *readers[i]);
                readNTAG(*readers[i]);
                Serial.println("Card halted and crypto stopped");
            }
            SPI.endTransaction();
            digitalWrite(readerConfigs[i].csPin, HIGH);
            Serial.print("CS pin "); Serial.print(readerConfigs[i].csPin); Serial.println(" deselected");
            delay(10);
        } else {
            Serial.print("Reader "); Serial.print(i); Serial.println(" is disabled or not initialized");
        }
    }
    Serial.println("NFCManagerV2::pollReaders() complete");
    delay(100); // Breathing room between poll cycles
}

void NFCManagerV2::printUid(const char* label, MFRC522& reader) {
    Serial.print("printUid called: ");
    Serial.print(label);
    for (byte i = 0; i < reader.uid.size; i++) {
        Serial.print(' ');
        Serial.print(reader.uid.uidByte[i], HEX);
    }
    Serial.println();
    Serial.println("printUid complete");
}

