#include <SPI.h>
#include <MFRC522.h>

//---------------------------------------------------------------------------
// Compile‐time check: require MFRC522 library version 1.4.12 exactly
// #if !defined(MFRC522_VERSION) || (MFRC522_VERSION != 0x010412)
//   #error "This sketch requires MFRC522 library version 1.4.12"
// #endif
// //---------------------------------------------------------------------------

#define SS_PIN      15    // change to your CS pin
#define RST_PIN      0    // RST tied to 3.3 V
#define UUID_VALUE   1    // the one‐number UUID you want to store

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); 
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH); 
  Serial.begin(115200);
  while (!Serial);

  SPI.begin();                      // VSPI on ESP32: SCK=18,MISO=19,MOSI=23
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);      // deselect

  rfid.PCD_Init();
  Serial.println("Ready: place an Ultralight tag to write UUID.");
}

void loop() {
  // wait for a new tag
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(100);
    return;
  }

  // check that it really is Ultralight (v1.4.12 lumps UL & UL-C under one constant)
  MFRC522::PICC_Type type = rfid.PICC_GetType(rfid.uid.sak);
  if (type != MFRC522::PICC_TYPE_MIFARE_UL) {
    Serial.println("✗ Not an Ultralight tag");
  }
  else {
    // print the factory UID
    Serial.print("Tag UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) Serial.print('0');
      Serial.print(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) Serial.print(':');
    }
    Serial.println();

    // pack our single‐number UUID into 4 bytes
    uint32_t uuid = UUID_VALUE;
    byte buf[4] = {
      byte((uuid >> 24) & 0xFF),
      byte((uuid >> 16) & 0xFF),
      byte((uuid >>  8) & 0xFF),
      byte((uuid      ) & 0xFF)
    };

    // write it to page 4 (first user page)
    MFRC522::StatusCode status = rfid.MIFARE_Write(4, buf, 4);
    if (status == MFRC522::STATUS_OK) {
      Serial.printf("✓ Wrote UUID = %u to page 4\n", uuid);
    } else {
      Serial.print("✗ Write failed: ");
      Serial.println(rfid.GetStatusCodeName(status));
    }
  }

  // cleanup and ready for next tag
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1000);
}
