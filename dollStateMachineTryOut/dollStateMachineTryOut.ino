#include <SPI.h>
#include <MFRC522.h>
#include <DFRobotDFPlayerMini.h>
#include <Adafruit_NeoPixel.h>
#include <HardwareSerial.h>

// ——— RFID readers ———————————————————————————————————————————————————
#define SS_PIN1   15   // medicine reader
#define SS_PIN2    4   // ear-check reader #1
#define SS_PIN3    5   // ear-check reader #2
#define RST_PIN    0

MFRC522 reader1(SS_PIN1, RST_PIN);
MFRC522 reader2(SS_PIN2, RST_PIN);
MFRC522 reader3(SS_PIN3, RST_PIN);

byte lastUID1[4] = {0};
byte lastUID2[4] = {0};
byte lastUID3[4] = {0};

// ——— Actuators & audio —————————————————————————————————————————————
#define ELECT_PIN   26   // electromagnet
#define NEO_PIN     22   // single NeoPixel
#define NEO_COUNT    1
#define LED_PIN     23   // yellow LED

// UART2 for DFPlayer Mini
#define DF_RX_PIN   16   // ESP32 RX2 ← DFPlayer TX
#define DF_TX_PIN   17   // ESP32 TX2 → DFPlayer RX

HardwareSerial dfSerial(2);
DFRobotDFPlayerMini player;

Adafruit_NeoPixel strip(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

// ——— Illness state machine ————————————————————————————————————————
enum Illness { HEALTHY, FEVER, EAR_INF };
Illness currentIllness;

// UIDs (first 4 bytes) to match against
const byte FEVER_MED_UID[4] = {0x04, 0xBF, 0x36, 0xB2};
const byte EAR_MED_UID[4]   = {0x04, 0xA2, 0x34, 0xB2};
const byte EAR_CHECK_UID[4] = {0x04, 0x59, 0x3C, 0xB2};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // —— NFC init ——
  SPI.begin();  // VSPI: SCK=18, MISO=19, MOSI=23
  pinMode(SS_PIN1, OUTPUT); digitalWrite(SS_PIN1, HIGH);
  pinMode(SS_PIN2, OUTPUT); digitalWrite(SS_PIN2, HIGH);
  pinMode(SS_PIN3, OUTPUT); digitalWrite(SS_PIN3, HIGH);
  reader1.PCD_Init();
  reader2.PCD_Init();
  reader3.PCD_Init();
  Serial.println("Ready: three-reader setup on default SPI");

  // —— Actuators init ——
  pinMode(ELECT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  strip.begin();
  strip.show();  // all off

  // —— DFPlayer init on UART2 ——
  dfSerial.begin(9600, SERIAL_8N1, DF_RX_PIN, DF_TX_PIN);
  Serial.printf("DFPlayer UART2 on RX=%d, TX=%d\n", DF_RX_PIN, DF_TX_PIN);
  Serial.println("Attempting DFPlayer.begin()...");
  if (!player.begin(dfSerial)) {
    Serial.println("▶️ player.begin() FAILED!");
    unsigned long deadline = millis() + 2000;
    while (millis() < deadline) {
      if (dfSerial.available()) {
        Serial.printf("  0x%02X ", dfSerial.read());
      }
    }
    Serial.println("\n— end of dump —");
    Serial.println("Check wiring, voltage, and levels.");
    while (true) delay(0);
  }
  Serial.println("✅ DFPlayer initialized!");
  player.volume(20);

  // —— Start with a fever by default ——
  currentIllness = FEVER;
  enterState(currentIllness);
}

void loop() {
  // poll all three readers
  pollReader(reader1, SS_PIN1, lastUID1, 1);
  pollReader(reader2, SS_PIN2, lastUID2, 2);
  pollReader(reader3, SS_PIN3, lastUID3, 3);
  delay(50);
}

// ——— your three-reader NFC polling/reporting ——————————————————————
void pollReader(MFRC522 &rfid, byte ssPin, byte *lastUID, int which) {
  digitalWrite(SS_PIN1, HIGH);
  digitalWrite(SS_PIN2, HIGH);
  digitalWrite(SS_PIN3, HIGH);
  digitalWrite(ssPin, LOW);

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    if (memcmp(rfid.uid.uidByte, lastUID, 4) != 0) {
      memcpy(lastUID, rfid.uid.uidByte, 4);
      reportTag(rfid, which);
      handleTag(which, rfid.uid.uidByte);
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  digitalWrite(ssPin, HIGH);
}

void reportTag(MFRC522 &rfid, int which) {
  auto type = rfid.PICC_GetType(rfid.uid.sak);
  Serial.printf("Reader %d tag: %s\n", which, rfid.PICC_GetTypeName(type));
  Serial.print("  UID:");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.printf(" %02X", rfid.uid.uidByte[i]);
  }
  Serial.println();
}

// ——— integrate NFC events into the state machine ——————————————————
void handleTag(int which, byte *uid) {
  // medicine reader
  if (which == 1) {
    if ((currentIllness == FEVER   && memcmp(uid, FEVER_MED_UID, 4) == 0) ||
        (currentIllness == EAR_INF && memcmp(uid, EAR_MED_UID,   4) == 0)) {
      cure();
    }
  }
  // ear-check readers (only when in EAR_INF)
  else if (currentIllness == EAR_INF) {
    if (which == 2 && memcmp(uid, EAR_CHECK_UID, 4) == 0) {
      strip.setPixelColor(0, strip.Color(255, 0, 0));  // red
      strip.show();
    }
    else if (which == 3 && memcmp(uid, EAR_CHECK_UID, 4) == 0) {
      digitalWrite(LED_PIN, HIGH);  // yellow
    }
  }
}

// ——— state transitions —————————————————————————————————————————————
void enterState(Illness st) {
  currentIllness = st;
  switch (st) {
    case FEVER:
      digitalWrite(ELECT_PIN, HIGH);
      strip.clear(); strip.show();
      digitalWrite(LED_PIN, LOW);
      player.play(1);  // crying
      break;

    case EAR_INF:
      digitalWrite(ELECT_PIN, LOW);
      strip.clear(); strip.show();
      digitalWrite(LED_PIN, LOW);
      player.play(1);
      break;

    case HEALTHY:
      cure();
      break;
  }
}

void cure() {
  currentIllness = HEALTHY;
  digitalWrite(ELECT_PIN, LOW);
  strip.clear(); strip.show();
  digitalWrite(LED_PIN, LOW);
  player.stop();
  Serial.println("Doll is healthy now!");
}
