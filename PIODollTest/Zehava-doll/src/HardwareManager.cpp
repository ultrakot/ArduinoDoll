#include "HardwareManager.h"

HardwareManager::HardwareManager() : currentTrack(1), buttonFlag(false), lastButtonState(HIGH), lastDebounceTime(0) {
    // Hardware initialization with 5 readers
    nfcReader = new NFCVSPI(15, 4, 5, 25, 27, 0); // SS1, SS2, SS3, SS4, SS5, RST
    mp3Player = new MP3Player(16, 17, 2);          // RX, TX, UART2
    irController = new IRController(26);            // IR LED on pin 14
    
    // NeoPixel initialization
    npRubAll = new Adafruit_NeoPixel(1, 33, NEO_GRB + NEO_KHZ800);
    npRubAll2 = new Adafruit_NeoPixel(1, 32, NEO_GRB + NEO_KHZ800);
    npEar = new Adafruit_NeoPixel(1, 22, NEO_GRB + NEO_KHZ800);
    npEar2 = new Adafruit_NeoPixel(1, 21, NEO_GRB + NEO_KHZ800);
}

HardwareManager::~HardwareManager() {
    delete nfcReader;
    delete mp3Player;
    delete irController;
    delete npRubAll;
    delete npRubAll2;
    delete npEar;
    delete npEar2;
}

void HardwareManager::initialize() {
    // Initialize NFC
    nfcReader->begin();
    Serial.println("VSPI NFC initialized with 5 readers");
    
    // Initialize MP3
    mp3Player->begin();
    delay(200); // Give MP3 player time to initialize
    Serial.println("MP3 Player initialized");
    
    // Initialize IR Controller
    irController->begin();
    irController->setInterval(1000); // Send every 1 second
    
    // Initialize NeoPixels
    npRubAll->begin(); 
    npRubAll2->begin(); 
    npEar->begin(); 
    npEar2->begin(); 
    
    // Clear all pixels
    turnOffAllLEDs();
    Serial.println("NeoPixels initialized");
    
    // Initialize button (pin 34 is input-only, requires external pull-up resistor)
    pinMode(BUTTON_PIN, INPUT);
    Serial.print("Button initialized on pin ");
    Serial.print(BUTTON_PIN);
    Serial.println(" (requires external pull-up resistor)");
    Serial.print("Button flag: ");
    Serial.println(buttonFlag ? "CLOSED" : "OPEN");
}

bool HardwareManager::detectCard(byte* uid, byte& size, int& reader) {
    if (nfcReader->poll(uid, size, reader)) {
        // Print detected UID to terminal
        Serial.print("VSPI UID detected: ");
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

void HardwareManager::playTrack(uint16_t track) {
    if (currentTrack != track) {
        if (mp3Player->isReady()) {
            mp3Player->playTrack(track);
            currentTrack = track;
            Serial.print("Playing track "); Serial.println(track);
        } else {
            Serial.println("WARNING: MP3 Player not ready yet");
        }
    }
}

void HardwareManager::stopAudio() {
    mp3Player->stop();
    Serial.println("Audio stopped");
}

void HardwareManager::setDiseaseSymptoms(int disease, int earColor) {
    turnOffAllLEDs(); // Start clean
    
    // IR should activate ONLY for:
    // - Fever (disease 4)
    // - Ear Infection (disease 3) with infected ear (earColor == 1)
    bool activateIR = (disease == 4) || (disease == 3 && earColor == 1);
    
    if (activateIR) {
        irController->start(0x34); // Start IR with fever/infection command
        Serial.println("IR activated for fever/ear infection");
    } else {
        irController->stop(); // Stop IR
    }
    
    if (disease == 0 || disease == 1 || disease == 4) { // Rubella, Allergy, or Fever
        if (disease == 4) {
            // Orange for fever (disease 4)
            npRubAll->setPixelColor(0, npRubAll->Color(255, 100, 0)); 
            npRubAll2->setPixelColor(0, npRubAll2->Color(255, 100, 0));
            Serial.println("Disease symptoms: Orange rash/spots (FEVER - IR active)");
        } else {
            // Red for Rubella/Allergy (no fever)
            npRubAll->setPixelColor(0, npRubAll->Color(255, 0, 0));
            npRubAll2->setPixelColor(0, npRubAll2->Color(255, 0, 0));
            Serial.println("Disease symptoms: Red rash/spots visible");
        }
        npRubAll->show();
        npRubAll2->show();
    } else if (disease == 2 || disease == 3) { // Ear Pain or Infection
        // Ear LEDs start OFF - will show during examination
        npEar->setPixelColor(0, 0); npEar->show();
        npEar2->setPixelColor(0, 0); npEar2->show();
        
        Serial.print("Ear disease active");
        if (disease == 3 && earColor == 1) {
            Serial.print(" (EAR INFECTION - IR active)");
        }
        Serial.print(" - Right ear: "); 
        Serial.println(earColor == 0 ? "Healthy" : "Infected");
        Serial.println("   (Use ear examination tool to diagnose)");
    }
}

void HardwareManager::setEarExaminationResult(int reader, bool infected) {
    if (reader == 1) { // reader2, SS pin 4 (right ear)
        if (!infected) {
            npEar->setPixelColor(0, npEar->Color(255,255,0)); // yellow - healthy ear
            Serial.println("Right ear examination: Healthy (Yellow light)");
        } else {
            npEar->setPixelColor(0, npEar->Color(255,0,0)); // red - infected ear
            Serial.println("Right ear examination: Infected! (Red light)");
        }
        npEar->show();
    }
    if (reader == 2) { // reader3, SS pin 5 (left ear)
        npEar2->setPixelColor(0, npEar2->Color(255,255,0)); // always yellow (healthy)
        npEar2->show();
        Serial.println("Left ear examination: Healthy (Yellow light)");
    }
}

void HardwareManager::turnOffAllLEDs() {
    npRubAll->setPixelColor(0, 0); npRubAll->show();
    npRubAll2->setPixelColor(0, 0); npRubAll2->show();
    npEar->setPixelColor(0, 0); npEar->show();
    npEar2->setPixelColor(0, 0); npEar2->show();
    
    // Stop IR when turning off all symptoms
    irController->stop();
}

void HardwareManager::updateIR() {
    irController->update();
}

void HardwareManager::testAllLEDs() {
    Serial.println("Testing all LEDs...");
    
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

void HardwareManager::updateButton() {
    // Read the button state
    // Pin 34 is input-only, needs external pull-up resistor
    // Button pressed = LOW (when button connects pin to GND)
    // Button released = HIGH (via external pull-up resistor)
    bool reading = digitalRead(BUTTON_PIN);
    
    // Check if button state changed
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    // If enough time has passed since last change, consider it a real change
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        // If button was just pressed (went from HIGH to LOW)
        if (reading == LOW && lastButtonState == HIGH) {
            // Toggle the flag
            buttonFlag = !buttonFlag;
            
            // Print the new state
            Serial.print("Button pressed - Flag: ");
            Serial.println(buttonFlag ? "CLOSED" : "OPEN");
        }
    }
    
    lastButtonState = reading;
}