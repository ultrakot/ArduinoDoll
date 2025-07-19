#include <SPI.h>
#include <MFRC522.h>

// CS pins for the three readers:
#define SS_PIN1   15
#define SS_PIN2   4
#define SS_PIN3   5

// All share the same RST (tied to 3.3 V)
#define RST_PIN    0

MFRC522 reader1(SS_PIN1, RST_PIN);
MFRC522 reader2(SS_PIN2, RST_PIN);
MFRC522 reader3(SS_PIN3, RST_PIN);

// Track last seen UIDs so we only report new tags
byte lastUID1[4] = {0};
byte lastUID2[4] = {0};
byte lastUID3[4] = {0};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // start default SPI (VSPI on ESP32: SCK=18, MISO=19, MOSI=23)
  SPI.begin();

  // configure all CS pins as outputs, idle HIGH (deselected)
  pinMode(SS_PIN1, OUTPUT); digitalWrite(SS_PIN1, HIGH);
  pinMode(SS_PIN2, OUTPUT); digitalWrite(SS_PIN2, HIGH);
  pinMode(SS_PIN3, OUTPUT); digitalWrite(SS_PIN3, HIGH);

  // initialize each reader
  reader1.PCD_Init();
  reader2.PCD_Init();
  reader3.PCD_Init();

  Serial.println("Ready: three-reader setup on default SPI");
}

void loop() {
  pollReader(reader1, SS_PIN1, lastUID1, 1);
  pollReader(reader2, SS_PIN2, lastUID2, 2);
  pollReader(reader3, SS_PIN3, lastUID3, 3);

  // small delay so you don’t hammer the bus too hard
  delay(50);
}

void pollReader(MFRC522 &rfid, byte ssPin, byte *lastUID, int which) {
  // deselect all, then select this one
  digitalWrite(SS_PIN1, HIGH);
  digitalWrite(SS_PIN2, HIGH);
  digitalWrite(SS_PIN3, HIGH);
  digitalWrite(ssPin, LOW);

  // check for a new tag
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    if (memcmp(rfid.uid.uidByte, lastUID, 4) != 0) {
      memcpy(lastUID, rfid.uid.uidByte, 4);
      reportTag(rfid, which);
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // deselect again
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

  // Insert your page/block read logic here if you need it...
}
