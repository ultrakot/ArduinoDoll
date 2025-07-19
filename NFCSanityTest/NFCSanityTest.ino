#include <SPI.h>
#include <MFRC522.h>

// ——— BUS 1 (VSPI) ——— SCLK=18, MISO=19, MOSI=23
const uint8_t BUS1_SCLK = 18, BUS1_MISO = 19, BUS1_MOSI = 23;
const uint8_t CS1 = 5, CS2 = 4, CS3 = 15;

// ——— BUS 2 (HSPI) ——— SCLK=14, MISO=12, MOSI=13
const uint8_t BUS2_SCLK = 14, BUS2_MISO = 12, BUS2_MOSI = 13;
const uint8_t CS4 = 25, CS5 = 27;

// tie all reader RST pins to 3.3 V and give a “dummy” here
const uint8_t RST = 22;

// instantiate each reader with (CS, RST)
MFRC522 rdr1(CS1, RST), rdr2(CS2, RST), rdr3(CS3, RST),
        rdr4(CS4, RST), rdr5(CS5, RST);

// array for easy looping
MFRC522* readers[5] = { &rdr1, &rdr2, &rdr3, &rdr4, &rdr5 };

void setup() {
  Serial.begin(115200);
  // init bus 1 readers
  SPI.begin(BUS1_SCLK, BUS1_MISO, BUS1_MOSI);
  rdr1.PCD_Init();
  rdr2.PCD_Init();
  rdr3.PCD_Init();
  // init bus 2 readers
  SPI.begin(BUS2_SCLK, BUS2_MISO, BUS2_MOSI);
  rdr4.PCD_Init();
  rdr5.PCD_Init();
  Serial.println("5 NFC readers ready");
}

void loop() {
  for (uint8_t i = 0; i < 5; i++) {
    // switch to the correct SPI bus
    if (i < 3)
      SPI.begin(BUS1_SCLK, BUS1_MISO, BUS1_MOSI);
    else
      SPI.begin(BUS2_SCLK, BUS2_MISO, BUS2_MOSI);

    MFRC522 &r = *readers[i];
    // if a tag appears and we can read its serial:
    if (r.PICC_IsNewCardPresent() && r.PICC_ReadCardSerial()) {
      Serial.print("R"); Serial.print(i + 1);
      Serial.print(" UID:");
      for (byte b = 0; b < r.uid.size; b++) {
        if (r.uid.uidByte[b] < 0x10) Serial.print(" 0");
        else                          Serial.print(' ');
        Serial.print(r.uid.uidByte[b], HEX);
      }
      Serial.println();
      // done with this tag
      r.PICC_HaltA();
      r.PCD_StopCrypto1();
    }
  }
  delay(100);
}
