#include <SPI.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

// --- SPI buses ---
SPIClass hspi(HSPI);  // HSPI: SCK=14, MISO=12, MOSI=13
SPIClass vspi(VSPI);  // VSPI: SCK=18, MISO=19, MOSI=23

// --- Chip Selects ---
#define CS_HSPI 25
#define CS_VSPI 15  // if flaky, try 5 or 27

// --- Optional: set per-bus SPI speeds ---
const SPISettings SPISET_HSPI(8000000, MSBFIRST, SPI_MODE0); // 8 MHz
const SPISettings SPISET_VSPI(4000000, MSBFIRST, SPI_MODE0); // 4 MHz (safer)

// --- HSPI reader objects ---
MFRC522DriverPinSimple ss_h(CS_HSPI);
MFRC522DriverSPI       drv_h(ss_h, hspi, SPISET_HSPI);
MFRC522                rfid_h(drv_h);

// --- VSPI reader objects ---
MFRC522DriverPinSimple ss_v(CS_VSPI);
MFRC522DriverSPI       drv_v(ss_v, vspi, SPISET_VSPI);
MFRC522                rfid_v(drv_v);

static void initReader(MFRC522& rfid, MFRC522DriverSPI& drv, const char* name) {
  rfid.PCD_Init();
  delay(50);
  uint8_t ver = drv.PCD_ReadRegister(MFRC522::PCD_Register::VersionReg);
  Serial.print(name); Serial.print(" version: 0x"); Serial.println(ver, HEX);
  if (ver == 0x00) Serial.println("❌ comm failure – check wiring/power");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // Don’t pass CS into begin(); the driver controls CS
  hspi.begin(14, 12, 13);
  vspi.begin(18, 19, 23);

  pinMode(CS_HSPI, OUTPUT); digitalWrite(CS_HSPI, HIGH);
  pinMode(CS_VSPI, OUTPUT); digitalWrite(CS_VSPI, HIGH);

  initReader(rfid_h, drv_h, "HSPI");
  initReader(rfid_v, drv_v, "VSPI");
}

static void poll(MFRC522& rfid, const char* tag) {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  Serial.print(tag); Serial.print(" UID:");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(' ');
    if (rfid.uid.uidByte[i] < 0x10) Serial.print('0');
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void loop() {
  poll(rfid_v, "VSPI");
  poll(rfid_h, "HSPI");
  delay(50);
}
