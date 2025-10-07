#include "HardwareManager.h"

HardwareManager::HardwareManager() : currentTrack(1) {
    // Hardware initialization with 5 readers
    nfcReader = new NFCVSPI(15, 4, 5, 25, 27, 0); // SS1, SS2, SS3, SS4, SS5, RST
    mp3Player = new MP3Player(16, 17, 2);          // RX, TX, UART2
    
    // NeoPixel initialization
    npRubAll = new Adafruit_NeoPixel(1, 33, NEO_GRB + NEO_KHZ800);
    npRubAll2 = new Adafruit_NeoPixel(1, 32, NEO_GRB + NEO_KHZ800);
    npEar = new Adafruit_NeoPixel(1, 22, NEO_GRB + NEO_KHZ800);
    npEar2 = new Adafruit_NeoPixel(1, 21, NEO_GRB + NEO_KHZ800);
}

HardwareManager::~HardwareManager() {
    delete nfcReader;
    delete mp3Player;
    delete npRubAll;
    delete npRubAll2;
    delete npEar;
    delete npEar2;
}

void HardwareManager::initialize() {
    // Initialize NFC
    nfcReader->begin();
    Serial.println("📡 VSPI NFC initialized with 5 readers");
    
    // Initialize MP3
    mp3Player->begin();
    Serial.println("🎵 MP3 Player initialized");
    
    // Initialize NeoPixels
    npRubAll->begin(); 
    npRubAll2->begin(); 
    npEar->begin(); 
    npEar2->begin(); 
    
    // Clear all pixels
    turnOffAllLEDs();
    Serial.println("💡 NeoPixels initialized");
}

bool HardwareManager::detectCard(byte* uid, byte& size, int& reader) {
    if (nfcReader->poll(uid, size, reader)) {
        // Print detected UID to terminal
        Serial.print("🔍 VSPI UID detected: ");
        for (byte i = 0; i < size; i++) {
            if (uid[i] < 0x10) Serial.print("0");
            Serial.print(uid[i], HEX);
            if (i < size - 1) Serial.print(" ");
        }
        Serial.print(" ("); Serial.print(size); Serial.print(" bytes) on reader "); Serial.println(reader + 1);
        return true;
    }
    return false;
}

void HardwareManager::playTrack(uint16_t track, bool forceRetry) {
    Serial.print("▶️ playTrack called (track="); Serial.print(track);
    Serial.print(", force="); Serial.print(forceRetry);
    Serial.println(")");

    // If not forcing retry and the requested track is already the current one, do nothing
    if (!forceRetry && currentTrack == track) {
        Serial.print("▶️ Already playing track "); Serial.println(track);
        return;
    }

    if (forceRetry) {
        // Try a stop+play sequence to force the module to switch
        mp3Player->stop();
        delay(80);
    }

    mp3Player->playTrack(track);
    currentTrack = track;
    Serial.print("▶️ Now playing track "); Serial.println(track);
}

void HardwareManager::stopAudio() {
    mp3Player->stop();
    Serial.println("⏹️ Audio stopped");
}

void HardwareManager::setDiseaseSymptoms(int disease, int earColor) {
    turnOffAllLEDs(); // Start clean
    
    if (disease == 0 || disease == 1 || disease == 4) { // Rubella, Allergy, or Fever
        npRubAll->setPixelColor(0, npRubAll->Color(255,0,0)); npRubAll->show();
        npRubAll2->setPixelColor(0, npRubAll2->Color(255,0,0)); npRubAll2->show();
        Serial.println("🔴 Disease symptoms: Red rash/spots visible");
    } else if (disease == 2 || disease == 3) { // Ear Pain or Infection
        // Ear LEDs start OFF - will show during examination
        npEar->setPixelColor(0, 0); npEar->show();
        npEar2->setPixelColor(0, 0); npEar2->show();
        
        Serial.print("👂 Ear disease active - ");
        Serial.print("Right ear: "); Serial.println(earColor == 0 ? "Healthy" : "Infected");
        Serial.println("   (Use ear examination tool to diagnose)");
    }
}

void HardwareManager::setEarExaminationResult(int reader, bool infected) {
    if (reader == 1) { // reader2, SS pin 4 (right ear)
        if (!infected) {
            npEar->setPixelColor(0, npEar->Color(255,255,0)); // yellow - healthy ear
            Serial.println("👂 Right ear examination: Healthy (Yellow light)");
        } else {
            npEar->setPixelColor(0, npEar->Color(255,0,0)); // red - infected ear
            Serial.println("👂 Right ear examination: Infected! (Red light)");
        }
        npEar->show();
    }
    if (reader == 2) { // reader3, SS pin 5 (left ear)
        npEar2->setPixelColor(0, npEar2->Color(255,255,0)); // always yellow (healthy)
        npEar2->show();
        Serial.println("👂 Left ear examination: Healthy (Yellow light)");
    }
}

void HardwareManager::turnOffAllLEDs() {
    npRubAll->setPixelColor(0, 0); npRubAll->show();
    npRubAll2->setPixelColor(0, 0); npRubAll2->show();
    npEar->setPixelColor(0, 0); npEar->show();
    npEar2->setPixelColor(0, 0); npEar2->show();
}

void HardwareManager::testAllLEDs() {
    Serial.println("🧪 Testing all LEDs...");
    
    // Test each LED individually
    Serial.println("Testing npRubAll (pin 33) - RED");
    npRubAll->setPixelColor(0, npRubAll->Color(255,0,0)); 
    npRubAll->show();
    delay(500);
    
    Serial.println("Testing npRubAll2 (pin 32) - GREEN");
    npRubAll2->setPixelColor(0, npRubAll2->Color(0,255,0)); 
    npRubAll2->show();
    delay(500);
    
    Serial.println("Testing npEar (pin 22) - BLUE");
    npEar->setPixelColor(0, npEar->Color(0,0,255)); 
    npEar->show();
    delay(500);
    
    Serial.println("Testing npEar2 (pin 21) - YELLOW");
    npEar2->setPixelColor(0, npEar2->Color(255,255,0)); 
    npEar2->show();
    delay(500);
    
    // All on white
    Serial.println("All LEDs WHITE");
    npRubAll->setPixelColor(0, npRubAll->Color(255,255,255));
    npRubAll2->setPixelColor(0, npRubAll2->Color(255,255,255));
    npEar->setPixelColor(0, npEar->Color(255,255,255));
    npEar2->setPixelColor(0, npEar2->Color(255,255,255));
    npRubAll->show(); npRubAll2->show(); npEar->show(); npEar2->show();
    delay(1000);
    
    turnOffAllLEDs();
    Serial.println("LED test complete");
}