#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  25
#define RST_PIN -1   // tie RST on module to 3.3 V

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Remap SPI to HSPI pins: SCK=14, MISO=12, MOSI=13, CS=25
  SPI.begin(14 /*SCK*/, 12 /*MISO*/, 13 /*MOSI*/, SS_PIN);
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);

  rfid.PCD_Init();
  delay(50);

  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
  Serial.print(F("MFRC522 version: 0x"));
  Serial.println(version, HEX);
  if (version == 0x00) {
    Serial.println(F("❌ Communication failure – check wiring & power"));
  }
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() ||
      !rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("UID:"));
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(' ');
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500);
}
