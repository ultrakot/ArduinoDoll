#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN   15   // your CS pin
#define RST_PIN   0   // dummy—reader’s RST tied to 3.3 V

MFRC522 rfid(SS_PIN, RST_PIN);
byte lastUID[4] = {0};

void setup() {
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); 
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH); 
  Serial.begin(115200);
  SPI.begin();           
  rfid.PCD_Init();       
  Serial.println("Ready: MIFARE Classic + NTAG (Ultralight)");
}

void loop() {
  // bail if no tag or failed to read UID
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  // grab the type
  auto type = rfid.PICC_GetType(rfid.uid.sak);
  Serial.print("Detected: ");
  Serial.println(rfid.PICC_GetTypeName(type));

  // only act on new tags
  if (memcmp(rfid.uid.uidByte, lastUID, 4) == 0) {
    Serial.println("• same tag, skipping");
    goto CLEANUP;
  }
  memcpy(lastUID, rfid.uid.uidByte, 4);

  // print UID
  Serial.print("UID (hex): ");
  printArray(rfid.uid.uidByte, rfid.uid.size, HEX);
  Serial.print("\nUID (dec): ");
  printArray(rfid.uid.uidByte, rfid.uid.size, DEC);
  Serial.println();

  if (type == MFRC522::PICC_TYPE_MIFARE_UL) {
    // NTAG/MIFARE Ultralight: no auth, just read pages
    Serial.println("→ NTAG pages 4–7:");
    for (byte page = 4; page <= 7; page++) {
      byte buf[18], sz = sizeof(buf);
      auto status = rfid.MIFARE_Read(page, buf, &sz);
      if (status != MFRC522::STATUS_OK) {
        Serial.print("  Read page "); Serial.print(page);
        Serial.print(" failed: "); 
        Serial.println(rfid.GetStatusCodeName(status));
      } else {
        Serial.print("  P"); Serial.print(page); Serial.print(":");
        for (byte j = 0; j < 4; j++) {
          if (buf[j] < 0x10) Serial.print(" 0");
          else               Serial.print(' ');
          Serial.print(buf[j], HEX);
        }
        Serial.println();
      }
    }
  }
  else if (type == MFRC522::PICC_TYPE_MIFARE_1K ||
           type == MFRC522::PICC_TYPE_MIFARE_4K ||
           type == MFRC522::PICC_TYPE_MIFARE_MINI) {
    // MIFARE Classic: authenticate & read block 4
    MFRC522::MIFARE_Key keyA = {{ 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF }};
    byte block = 4;
    auto st = rfid.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &keyA, &rfid.uid
    );
    if (st != MFRC522::STATUS_OK) {
      Serial.print("  Auth failed: ");
      Serial.println(rfid.GetStatusCodeName(st));
    } else {
      byte buf[18], sz = sizeof(buf);
      st = rfid.MIFARE_Read(block, buf, &sz);
      if (st != MFRC522::STATUS_OK) {
        Serial.print("  Read blk4 failed: ");
        Serial.println(rfid.GetStatusCodeName(st));
      } else {
        Serial.print("  Block 4:");
        for (byte j = 0; j < 16; j++) {
          if (buf[j] < 0x10) Serial.print(" 0");
          else               Serial.print(' ');
          Serial.print(buf[j], HEX);
        }
        Serial.println();
      }
    }
  }
  else {
    Serial.println("• unsupported tag type");
  }

CLEANUP:
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(150);
}

void printArray(byte *buf, byte len, byte base) {
  for (byte i = 0; i < len; i++) {
    if (base == HEX && buf[i] < 0x10) Serial.print('0');
    Serial.print(buf[i], base);
    if (i < len - 1) Serial.print(' ');
  }
}
