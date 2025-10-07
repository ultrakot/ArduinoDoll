#include "GameLogic.h"

GameLogic::GameLogic(HardwareManager* hw, DiseaseManager* dm) 
    : currentState(STATE_INIT), stateTimer(0), hardware(hw), diseaseManager(dm) {
}

void GameLogic::initialize() {
    hardware->setDiseaseSymptoms(diseaseManager->getCurrentDisease(), diseaseManager->getEarColor());
    
    // Audio disabled for polling test: would play track 1 here
    Serial.println("😷 (TEST MODE) Audio disabled - would play crying sound (track 1)");
}

void GameLogic::update() {
    switch (currentState) {
        case STATE_INIT:
            // Initialize is already called from main.cpp, just change state
            changeState(STATE_SICK);
            break;
            
        case STATE_SICK:
            handleSickState();
            break;
            
        case STATE_REACTING_TO_MEDICINE:
            handleReactingToMedicineState();
            break;
            
        case STATE_EAR_TEST:
            handleEarTestState();
            break;
            
        case STATE_HEALING:
            handleHealingState();
            break;
            
        case STATE_GAME_OVER:
            handleGameOverState();
            break;
    }
}

void GameLogic::handleSickState() {
    byte foundUID[10];
    byte foundSize;
    int readerIndex;
    
    if (checkForCard(foundUID, foundSize, readerIndex)) {
        Serial.print("💊 Card detected on reader "); Serial.println(readerIndex + 1);

        // Identify medicine and play its dedicated track (no state changes)
        UIDManager::MedicineType medicine = UIDManager::identifyMedicine(foundUID, foundSize);
        if (medicine != UIDManager::UNKNOWN) {
            uint16_t track = UIDManager::getTrackForMedicine(medicine);
                // Audio removed for polling test: only print the track number that would be played
                Serial.print("💊 (TEST MODE) Medicine "); Serial.print(UIDManager::getMedicineName(medicine));
                Serial.print(" would play track "); Serial.println(track);
        } else {
            Serial.println("❓ Unknown UID - no medicine track");
        }
    }
    // Note: When no medicine is present, doll continues crying (track 1 set in initialize)
}

void GameLogic::handleReactingToMedicineState() {
    byte foundUID[10];
    byte foundSize;
    int readerIndex;
    
    if (!checkForCard(foundUID, foundSize, readerIndex)) {
    // Medicine removed, doll goes back to being sick
    Serial.println("💊 Medicine removed - doll becomes sick again");
    Serial.println("😷 (TEST MODE) Would resume crying sound (track 1)");
    changeState(STATE_SICK);
    }
    // Continue reacting while medicine is present
}

void GameLogic::handleEarTestState() {
    byte foundUID[10];
    byte foundSize;
    int readerIndex;
    
    if (!checkForCard(foundUID, foundSize, readerIndex)) {
    // Examination tool removed, go back to being sick
    Serial.println("👂 Ear examination complete - doll is still sick");
    Serial.println("😷 (TEST MODE) Would resume crying sound (track 1)");
    changeState(STATE_SICK);
    }
    // NeoPixels remain on while examination is happening
}

void GameLogic::handleHealingState() {
    Serial.print("💊 Healing "); Serial.print(diseaseManager->getCurrentDiseaseName());
    Serial.println(" with correct medicine!");
    
    uint16_t healingTrack = diseaseManager->getHealingTrack();
    // Audio removed for polling test: only print the healing track number
    Serial.print("😊 (TEST MODE) Would play healing sound (track "); 
    Serial.print(healingTrack); Serial.println(")");
    
    stateTimer = millis();
    
    // Wait for healing sound to finish
    while (millis() - stateTimer < 5000) {
        delay(100);
    }
    
    Serial.println("✨ (TEST MODE) Doll is completely healed! (would stop audio)");
    changeState(STATE_GAME_OVER);
}

void GameLogic::handleGameOverState() {
    hardware->turnOffAllLEDs();
    Serial.println("🎉 Doll is healthy and happy! All symptoms gone - game complete!");
    Serial.println("⏹️ Program ended.");
    
    // Stop the program - infinite loop doing nothing
    while(true) {
        delay(1000);
    }
}

bool GameLogic::checkForCard(byte* uid, byte& size, int& reader) {
    return hardware->detectCard(uid, size, reader);
}

void GameLogic::handleEarTestDetection(int readerIndex) {
    bool infected = (diseaseManager->getEarColor() == 1);
    hardware->setEarExaminationResult(readerIndex, infected);
}

void GameLogic::changeState(GameState newState) {
    const char* stateNames[] = {"INIT", "SICK", "REACTING_TO_MEDICINE", "EAR_TEST", "HEALING", "GAME_OVER"};
    Serial.print("🔄 State: ");
    Serial.print(stateNames[currentState]);
    Serial.print(" -> ");
    Serial.println(stateNames[newState]);
    
    currentState = newState;
    stateTimer = millis();
}