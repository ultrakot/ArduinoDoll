#include <SPI.h>
#include <MFRC522.h>

// Chip-select pins for Reader 1 and Reader 2
#define SS_PIN1 25
#define SS_PIN2 27
#define RST_PIN  -1   // RST tied to 3.3 V

// Two MFRC522 instances
MFRC522 reader1(SS_PIN1, RST_PIN);
MFRC522 reader2(SS_PIN2, RST_PIN);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize HSPI: SCK=14, MISO=12, MOSI=13, and use SS_PIN1 as the default SS
  SPI.begin(14 /* SCK */, 12 /* MISO */, 13 /* MOSI */, SS_PIN1);

  // Configure both CS pins and deselect both readers
  pinMode(SS_PIN1, OUTPUT);
  pinMode(SS_PIN2, OUTPUT);
  digitalWrite(SS_PIN1, HIGH);
  digitalWrite(SS_PIN2, HIGH);

  // Initialize both readers
  reader1.PCD_Init();
  delay(50);
  reader2.PCD_Init();
  delay(50);

  // Read and print firmware versions for both
  byte ver1 = reader1.PCD_ReadRegister(reader1.VersionReg);
  Serial.print(F("Reader1 MFRC522 version: 0x"));
  Serial.println(ver1, HEX);
  if (ver1 == 0x00) {
    Serial.println(F("❌ Reader1 communication failure – check wiring & power"));
  }

  byte ver2 = reader2.PCD_ReadRegister(reader2.VersionReg);
  Serial.print(F("Reader2 MFRC522 version: 0x"));
  Serial.println(ver2, HEX);
  if (ver2 == 0x00) {
    Serial.println(F("❌ Reader2 communication failure – check wiring & power"));
  }
}

void loop() {
  // -- Reader 1 --
  if ( reader1.PICC_IsNewCardPresent() && reader1.PICC_ReadCardSerial() ) {
    Serial.print(F("Reader1 UID:"));
    for (byte i = 0; i < reader1.uid.size; i++) {
      Serial.print(' ');
      Serial.print(reader1.uid.uidByte[i], HEX);
    }
    Serial.println();
    reader1.PICC_HaltA();
    reader1.PCD_StopCrypto1();
    delay(200);  // small debounce
  }

  // -- Reader 2 --
  if ( reader2.PICC_IsNewCardPresent() && reader2.PICC_ReadCardSerial() ) {
    Serial.print(F("Reader2 UID:"));
    for (byte i = 0; i < reader2.uid.size; i++) {
      Serial.print(' ');
      Serial.print(reader2.uid.uidByte[i], HEX);
    }
    Serial.println();
    reader2.PICC_HaltA();
    reader2.PCD_StopCrypto1();
    delay(200);
  }
}
