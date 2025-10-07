#include <SPI.h>
#include <MFRC522.h>

// === CS pins for the five readers ===
#define SS_PIN1  15
#define SS_PIN2  4
#define SS_PIN3  5
#define SS_PIN4  25
#define SS_PIN5  27

// All share the same RST (can be tied to 3.3V or a digital pin)
#define RST_PIN  0

MFRC522 reader1(SS_PIN1, RST_PIN);
MFRC522 reader2(SS_PIN2, RST_PIN);
MFRC522 reader3(SS_PIN3, RST_PIN);
MFRC522 reader4(SS_PIN4, RST_PIN);
MFRC522 reader5(SS_PIN5, RST_PIN);

// Track last seen UIDs so we only report new tags
byte lastUID1[4] = {0};
byte lastUID2[4] = {0};
byte lastUID3[4] = {0};
byte lastUID4[4] = {0};
byte lastUID5[4] = {0};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize SPI (default VSPI on ESP32: SCK=18, MISO=19, MOSI=23)
  SPI.begin();

  // Set all SS pins as outputs and deselect them
  int ssPins[] = {SS_PIN1, SS_PIN2, SS_PIN3, SS_PIN4, SS_PIN5};
  for (int i = 0; i < 5; i++) {
    pinMode(ssPins[i], OUTPUT);
    digitalWrite(ssPins[i], HIGH);
  }

  // Initialize all readers
  reader1.PCD_Init();
  reader2.PCD_Init();
  reader3.PCD_Init();
  reader4.PCD_Init();
  reader5.PCD_Init();

  Serial.println("Ready: five-reader setup on default SPI");
}

void loop() {
  pollReader(reader1, SS_PIN1, lastUID1, 1);
  pollReader(reader2, SS_PIN2, lastUID2, 2);
  pollReader(reader3, SS_PIN3, lastUID3, 3);
  pollReader(reader4, SS_PIN4, lastUID4, 4);
  pollReader(reader5, SS_PIN5, lastUID5, 5);

  delay(50); // Small delay to reduce SPI load
}

void pollReader(MFRC522 &rfid, byte ssPin, byte *lastUID, int which) {
  // Deselect all CS pins first
  digitalWrite(SS_PIN1, HIGH);
  digitalWrite(SS_PIN2, HIGH);
  digitalWrite(SS_PIN3, HIGH);
  digitalWrite(SS_PIN4, HIGH);
  digitalWrite(SS_PIN5, HIGH);

  // Select current reader
  digitalWrite(ssPin, LOW);

  // Try reading card directly (no PICC_IsNewCardPresent)
  if (rfid.PICC_ReadCardSerial()) {
    if (memcmp(rfid.uid.uidByte, lastUID, 4) != 0) {
      memcpy(lastUID, rfid.uid.uidByte, 4);
      reportTag(rfid, which);
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // Deselect current reader
  digitalWrite(ssPin, HIGH);
}

void reportTag(MFRC522 &rfid, int which) {
  auto type = rfid.PICC_GetType(rfid.uid.sak);
  Serial.printf("Reader %d tag: %s\n", which, rfid.PICC_GetTypeName(type));
  Serial.print("  UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print('0');
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(':');
  }
  Serial.println();
}
