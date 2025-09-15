#include <SPI.h>
#include <MFRC522.h>

// VSPI: SCK=18, MISO=19, MOSI=23
// HSPI: SCK=14, MISO=12, MOSI=13

// VSPI readers (3)
#define CS_V1 15
#define CS_V2 4
#define CS_V3 5

// HSPI readers (2)
#define CS_H1 25
#define CS_H2 27

#define RST_PIN -1

// 2-arg ctor only
MFRC522 V1(CS_V1, RST_PIN), V2(CS_V2, RST_PIN), V3(CS_V3, RST_PIN);
MFRC522 H1(CS_H1, RST_PIN), H2(CS_H2, RST_PIN);

enum Bus { BUS_NONE, BUS_VSPI, BUS_HSPI };
Bus currentBus = BUS_NONE;

// switch to VSPI with default SS
void useVSPI() {
  if (currentBus != BUS_VSPI) {
    SPI.end();
    SPI.begin(18, 19, 23, CS_V1);
    delay(2);
    currentBus = BUS_VSPI;
  }
}
// switch to HSPI with default SS
void useHSPI() {
  if (currentBus != BUS_HSPI) {
    SPI.end();
    SPI.begin(14, 12, 13, CS_H1);
    delay(2);
    currentBus = BUS_HSPI;
  }
}

inline void deselectAll(uint8_t a, uint8_t b, uint8_t c=255) {
  if (a!=255) digitalWrite(a,HIGH);
  if (b!=255) digitalWrite(b,HIGH);
  if (c!=255) digitalWrite(c,HIGH);
}

void setup() {
  Serial.begin(115200);
  while(!Serial){}

  // CS lines idle HIGH
  pinMode(CS_V1,OUTPUT); digitalWrite(CS_V1,HIGH);
  pinMode(CS_V2,OUTPUT); digitalWrite(CS_V2,HIGH);
  pinMode(CS_V3,OUTPUT); digitalWrite(CS_V3,HIGH);
  pinMode(CS_H1,OUTPUT); digitalWrite(CS_H1,HIGH);
  pinMode(CS_H2,OUTPUT); digitalWrite(CS_H2,HIGH);

  // Init VSPI group
  useVSPI();
  V1.PCD_Init(); delay(30);
  V2.PCD_Init(); delay(30);
  V3.PCD_Init(); delay(30);

  // Init HSPI group
  useHSPI();
  H1.PCD_Init(); delay(30);
  H2.PCD_Init(); delay(30);

  Serial.println(F("Ready: VSPI(3)+HSPI(2) with transactions & default SS"));
}

void reportTag(MFRC522 &rfid, char busTag, int idxOnBus) {
  auto type = rfid.PICC_GetType(rfid.uid.sak);
  Serial.print("Bus "); Serial.print(busTag);
  Serial.print(" Reader "); Serial.print(idxOnBus);
  Serial.print(" -> "); Serial.println(MFRC522::PICC_GetTypeName(type));
  Serial.print("  UID: ");
  for (byte i=0;i<rfid.uid.size;i++){
    if (rfid.uid.uidByte[i] < 0x10) Serial.print('0');
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i<rfid.uid.size-1) Serial.print(':');
  }
  Serial.println();
}

void pollReader(
  MFRC522 &rfid, uint8_t csThis,
  char busTag, int idxOnBus,
  uint8_t csA, uint8_t csB, uint8_t csC=255
) {
  // ensure only this device is active on the bus
  deselectAll(csA, csB, csC);
  digitalWrite(csThis, LOW);

  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

  bool ok = rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial();
  if (ok) {
    // always report every read, no dedup
    reportTag(rfid, busTag, idxOnBus);
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  SPI.endTransaction();
  digitalWrite(csThis, HIGH);
}

void loop() {
  // VSPI pass
  useVSPI();
  pollReader(V1, CS_V1, 'V', 1, CS_V1, CS_V2, CS_V3);
  pollReader(V2, CS_V2, 'V', 2, CS_V1, CS_V2, CS_V3);
  pollReader(V3, CS_V3, 'V', 3, CS_V1, CS_V2, CS_V3);

  // HSPI pass
  useHSPI();
  pollReader(H1, CS_H1, 'H', 1, CS_H1, CS_H2);
  pollReader(H2, CS_H2, 'H', 2, CS_H1, CS_H2);

  delay(20);
}
