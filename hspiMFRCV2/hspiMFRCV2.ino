#include <SPI.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

// -------- SPI bus: HSPI on ESP32 --------
// HSPI default pins: SCK=14, MISO=12, MOSI=13
SPIClass hspi(HSPI);

// -------- Reader wiring --------
#define SS_PIN   25  // CS
// RST tied to 3.3V (no RST pin needed with v2)

// v2 driver + reader objects
MFRC522DriverPinSimple ss(SS_PIN);
MFRC522DriverSPI driver(ss, hspi);
MFRC522 rfid(driver);

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* wait for USB */ }

  // Initialize HSPI with explicit pins (SCK, MISO, MOSI, SS)
  hspi.begin(14 /*SCK*/, 12 /*MISO*/, 13 /*MOSI*/, SS_PIN);

  // Optional: ensure CS is idle HIGH (driver handles CS internally anyway)
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);

  // Init reader
  rfid.PCD_Init();
  delay(50);

  // --- Version register read (v2: use the DRIVER, not rfid) ---
  uint8_t version = driver.PCD_ReadRegister(MFRC522::PCD_Register::VersionReg);
  Serial.print(F("MFRC522 version: 0x"));
  Serial.println(version, HEX);
  if (version == 0x00) {
    Serial.println(F("❌ Communication failure – check wiring & power"));
  } else if (version == 0x91 || version == 0x92) {
    Serial.println(F("✅ MFRC522 detected"));
  }
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("UID:"));
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(' ');
    if (rfid.uid.uidByte[i] < 0x10) Serial.print('0');
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500);
}
