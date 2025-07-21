#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

// ── PIN CONFIG ────────────────────────────────────────────────────────────
#define SS_PIN1     25   // HSPI CS for Reader 1
#define SS_PIN2     27   // HSPI CS for Reader 2
#define RST_PIN     -1   // RST tied to 3.3 V

#define PIXEL_PIN1  33   // “Illness” NeoPixel
#define PIXEL_PIN2  32   // “Allergy” NeoPixel

#define ELECTRO_PIN 26   // Electromagnet control (always HIGH)

#define DF_RX_PIN   16   // DFPlayer RX ← ESP32 TX2
#define DF_TX_PIN   17   // DFPlayer TX → ESP32 RX2

// ── HARDWARE OBJECTS ──────────────────────────────────────────────────────
MFRC522 reader1(SS_PIN1, RST_PIN);
MFRC522 reader2(SS_PIN2, RST_PIN);

Adafruit_NeoPixel strip1(1, PIXEL_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(1, PIXEL_PIN2, NEO_GRB + NEO_KHZ800);

HardwareSerial dfSerial(2);
DFRobotDFPlayerMini player;

// ── TARGET UID ─────────────────────────────────────────────────────────────
// Your card: 04 A2 34 B2 35 5E 80  (7 bytes)
const byte targetUID[7] = { 
  0x04, 0xA2, 0x34, 0xB2, 
  0x35, 0x5E, 0x80 
};

// ── STATE MACHINE ─────────────────────────────────────────────────────────
enum State { SICK, GOT_TREAT1, GOT_TREAT2, CURED };
State state = SICK;

// ── HELPERS ────────────────────────────────────────────────────────────────
void logUID(uint8_t readerNum, byte *uid, byte len) {
  Serial.print("Reader "); Serial.print(readerNum);
  Serial.print(" UID:");
  for (byte i = 0; i < len; i++) {
    if (uid[i] < 0x10) Serial.print('0');
    Serial.print(uid[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

bool compareUID(byte *readUID, byte len) {
  if (len != sizeof(targetUID)) return false;
  for (byte i = 0; i < len; i++)
    if (readUID[i] != targetUID[i]) return false;
  return true;
}

void playTreatmentSound() {
  player.play(2);          // one-off treatment jingle
  delay(4000);             // <-- adjust to match track length
  if (state != CURED) {
    player.loop(1);        // back to crying doll
  }
}

// ── STATE HANDLERS ────────────────────────────────────────────────────────
void enterSick() {
  state = SICK;
  strip1.setBrightness(25);  // ~30% of 255
  strip2.setBrightness(25);
  strip1.setPixelColor(0, strip1.Color(255,0,0));
  strip2.setPixelColor(0, strip2.Color(255,0,0));
  strip1.show();
  strip2.show();
  Serial.println("⦿ State=SICK: both LEDs RED, crying loop starts");
  player.loop(1);
}

void onTreatment1() {
  if (state == SICK) {
    state = GOT_TREAT1;
    Serial.println("⦿ State=GOT_TREAT1: illness LED OFF");
  } else if (state == GOT_TREAT2) {
    state = CURED;
    onCured();
    return;
  }
  strip2.clear(); strip2.show();
  playTreatmentSound();
}

void onTreatment2() {
  if (state == SICK) {
    state = GOT_TREAT2;
    Serial.println("⦿ State=GOT_TREAT2: allergy LED OFF");
  } else if (state == GOT_TREAT1) {
    state = CURED;
    onCured();
    return;
  }
  strip1.clear(); strip1.show();
  playTreatmentSound();
}

void onCured() {
  Serial.println("⦿ State=CURED: all LEDs OFF → stopping audio");
  player.stop();            // stop crying loop
  // electromagnet stays ON per your spec
}

// ── SETUP & LOOP ─────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  while (!Serial);

  // HSPI on VSPI pins: SCK=14, MISO=12, MOSI=13
  SPI.begin(14, 12, 13, SS_PIN1);
  pinMode(SS_PIN1, OUTPUT); digitalWrite(SS_PIN1, HIGH);
  pinMode(SS_PIN2, OUTPUT); digitalWrite(SS_PIN2, HIGH);
  reader1.PCD_Init(); delay(50);
  reader2.PCD_Init(); delay(50);

  // NeoPixels
  strip1.begin();
  strip2.begin();

  // Electromagnet always ON
  pinMode(ELECTRO_PIN, OUTPUT);
  digitalWrite(ELECTRO_PIN, HIGH);

  // DFPlayer init
  dfSerial.begin(9600, SERIAL_8N1, DF_RX_PIN, DF_TX_PIN);
  if (!player.begin(dfSerial)) {
    Serial.println("❌ DFPlayer Init failed!");
    while (true) delay(0);
  }
  player.volume(20);

  enterSick();
}

void loop() {
  // Reader 1
  if (reader1.PICC_IsNewCardPresent() &&
      reader1.PICC_ReadCardSerial()) {
    logUID(1, reader1.uid.uidByte, reader1.uid.size);
    reader1.PICC_HaltA();
    reader1.PCD_StopCrypto1();
    if (compareUID(reader1.uid.uidByte, reader1.uid.size)) {
      Serial.println("→ match on Reader 1");
      onTreatment1();
    }
    delay(200);
  }
  // Reader 2
  if (reader2.PICC_IsNewCardPresent() &&
      reader2.PICC_ReadCardSerial()) {
    logUID(2, reader2.uid.uidByte, reader2.uid.size);
    reader2.PICC_HaltA();
    reader2.PCD_StopCrypto1();
    if (compareUID(reader2.uid.uidByte, reader2.uid.size)) {
      Serial.println("→ match on Reader 2");
      onTreatment2();
    }
    delay(200);
  }
}
