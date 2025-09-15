#include "NFCVSPI.h"
#include "NFCHSPI.h"
#include "MP3Player.h"
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

NFCVSPI vspiNfc(15, 4, 5, 0); // SS1, SS2, SS3, RST
MP3Player mp3(16, 17, 2);     // RX, TX, UART2
const byte STOP_UID[4] = {0x34, 0x82, 0x36, 0x02}; // akamol
const byte UID_OINTMENT[7]   = {0x04, 0xA2, 0x34, 0xB2, 0x35, 0x5E, 0x80};
const byte UID_ANTIBIOTICS[7]= {0x04, 0xBF, 0x36, 0xB2, 0x35, 0x5E, 0x80};
const byte UID_EARDROPS[7]   = {0x04, 0xA1, 0x35, 0xB2, 0x35, 0x5E, 0x80};

// Disease stop UIDs and healing tracks
const byte RUBELLA_UID[4]     = {0x34, 0x82, 0x36, 0x02}; // akamol
const byte ALLERGY_UID[7]     = {0x04, 0xA2, 0x34, 0xB2, 0x35, 0x5E, 0x80}; // oinment
const byte EARPAIN_UID[7]     = {0x04, 0xA1, 0x35, 0xB2, 0x35, 0x5E, 0x80}; // eardrops 
const byte EARINFECTION_UID[7]= {0x04, 0xA1, 0x35, 0xB2, 0x35, 0x5E, 0x80}; // antibiotics
const byte FEVER_UID[4]       = {0x34, 0x82, 0x36, 0x02}; // akamol

struct UIDTrack {
    const byte* uid;
    byte size;
    uint16_t track;
};

UIDTrack tracks[] = {
    {UID_ANTIBIOTICS, 7, 4},
    {UID_EARDROPS,    7, 3},
    {UID_OINTMENT,    7, 2}
};
const int numTracks = sizeof(tracks)/sizeof(tracks[0]);

struct DiseaseStop {
    const byte* uid;
    byte size;
    uint16_t healTrack;
    const char* name;
};

DiseaseStop diseaseStops[] = {
    {RUBELLA_UID,      4, 5, "Rubella"},
    {ALLERGY_UID,      7, 4, "Allergy"},
    {EARPAIN_UID,      7, 2, "Ear Pain"},
    {EARINFECTION_UID, 7, 2, "Ear Infection"},
    {FEVER_UID,       4, 3, "Fever"}
};
const int numDiseases = sizeof(diseaseStops)/sizeof(diseaseStops[0]);

// NeoPixel setup
Adafruit_NeoPixel npRubAll(1, 33, NEO_GRB + NEO_KHZ800); // Rubella/Allergy
Adafruit_NeoPixel npRubAll2(1, 32, NEO_GRB + NEO_KHZ800); // Rubella/Allergy
Adafruit_NeoPixel npEar(1, 22, NEO_GRB + NEO_KHZ800);     // Ear Pain/Infection
Adafruit_NeoPixel npEar2(1, 21, NEO_GRB + NEO_KHZ800);    // Ear Pain/Infection

int activeDisease = -1;
const int EEPROM_ADDR = 0;
bool isPlaying = false;
bool stopAll = false;

void setup() {
    EEPROM.begin(4); // 4 bytes is enough for one int
    int diseaseIndex = EEPROM.read(EEPROM_ADDR);
    if (diseaseIndex < 0 || diseaseIndex >= numDiseases) diseaseIndex = 0;
    activeDisease = diseaseIndex;
    diseaseIndex++;
    if (diseaseIndex >= numDiseases) diseaseIndex = 0;
    EEPROM.write(EEPROM_ADDR, diseaseIndex);
    EEPROM.commit();
    vspiNfc.begin();
    mp3.begin();
    npRubAll.begin(); npRubAll.show();
    npRubAll2.begin(); npRubAll2.show();
    npEar.begin(); npEar.show();
    npEar2.begin(); npEar2.show();
    Serial.print("Active disease: "); Serial.println(diseaseStops[activeDisease].name);
    // Set NeoPixels for active disease
    if (activeDisease == 0 || activeDisease == 1) { // Rubella or Allergy
        npRubAll.setPixelColor(0, npRubAll.Color(255,0,0)); npRubAll.show();
        npRubAll2.setPixelColor(0, npRubAll2.Color(255,0,0)); npRubAll2.show();
    } else if (activeDisease == 2 || activeDisease == 3) { // Ear Pain or Infection
        npEar.setPixelColor(0, npEar.Color(255,255,0)); npEar.show();
        npEar2.setPixelColor(0, npEar2.Color(255,255,0)); npEar2.show();
    }
}

// Helper to compare UIDs
bool compareUID(const byte* a, byte aSize, const byte* b, byte bSize) {
    if (aSize != bSize) return false;
    for (byte i = 0; i < aSize; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

void loop() {
    if (stopAll) return;
    MFRC522* readers[3] = {vspiNfc._reader1, vspiNfc._reader2, vspiNfc._reader3};
    uint8_t ssPins[3] = {vspiNfc._ss1, vspiNfc._ss2, vspiNfc._ss3};
    static uint16_t currentTrack = 1;
    static unsigned long lastSwitch = 0;
    bool uidFound = false;
    for (int i = 0; i < 3; i++) {
        if (readers[i]) {
            digitalWrite(vspiNfc._ss1, HIGH);
            digitalWrite(vspiNfc._ss2, HIGH);
            digitalWrite(vspiNfc._ss3, HIGH);
            digitalWrite(ssPins[i], LOW);
            if (readers[i]->PICC_IsNewCardPresent() && readers[i]->PICC_ReadCardSerial()) {
                byte* uid = readers[i]->uid.uidByte;
                byte uidSize = readers[i]->uid.size;
                // Check for STOP UID
                // if (compareUID(uid, uidSize, STOP_UID, 4)) {
                //     mp3.playTrack(5); // Play stop UID sound
                //     Serial.println("▶️ Playing stop UID track 5");
                //     delay(3000); // Wait for sound to play (adjust as needed)
                //     mp3.stop();
                //     isPlaying = false;
                //     stopAll = true;
                //     Serial.println("⏹️ Stopped track and loop due to UID match (akamol)");
                //     return;
                // }
                // Check for healing UID for active disease
                if (compareUID(uid, uidSize, diseaseStops[activeDisease].uid, diseaseStops[activeDisease].size)) {
                    mp3.playTrack(diseaseStops[activeDisease].healTrack); // Play healing track
                    Serial.print("▶️ Healing: "); Serial.println(diseaseStops[activeDisease].name);
                    delay(3000); // Wait for sound to play (adjust as needed)
                    mp3.stop();
                    isPlaying = false;
                    stopAll = true;
                    // Turn off NeoPixels
                    npRubAll.setPixelColor(0, 0); npRubAll.show();
                    npRubAll2.setPixelColor(0, 0); npRubAll2.show();
                    npEar.setPixelColor(0, 0); npEar.show();
                    npEar2.setPixelColor(0, 0); npEar2.show();
                    Serial.println("⏹️ Disease healed, LEDs off, loop stopped.");
                    return;
                }
                // Check for other UIDs
                for (int t = 0; t < numTracks; t++) {
                    if (compareUID(uid, uidSize, tracks[t].uid, tracks[t].size)) {
                        if (currentTrack != tracks[t].track || !isPlaying) {
                            mp3.playTrack(tracks[t].track);
                            currentTrack = tracks[t].track;
                            isPlaying = true;
                            Serial.print("▶️ Playing track "); Serial.println(currentTrack);
                        }
                        uidFound = true;
                        break;
                    }
                }
                readers[i]->PICC_HaltA();
                readers[i]->PCD_StopCrypto1();
                delay(500);
            }
            digitalWrite(ssPins[i], HIGH);
        }
    }
    // If no UID found, return to track 1
    if (!uidFound && (currentTrack != 1 || !isPlaying)) {
        mp3.playTrack(1);
        currentTrack = 1;
        isPlaying = true;
        Serial.println("▶️ Waiting for card, playing track 1");
    }
    delay(100);
}

