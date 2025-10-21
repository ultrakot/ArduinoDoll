#include "GameLogic.h"

GameLogic::GameLogic(HardwareManager* hw, DiseaseManager* dm) 
    : currentState(STATE_INIT), stateTimer(0), hardware(hw), diseaseManager(dm) {
}

void GameLogic::initialize() {
    hardware->setDiseaseSymptoms(diseaseManager->getCurrentDisease(), diseaseManager->getEarColor());
    
    // Give MP3 player time to fully initialize
    delay(500);
    
    // Start with track 1 (sick/crying sound)
    hardware->playTrack(1);
    Serial.println("Doll is sick - playing crying sound (track 1)");
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
        Serial.print("Medicine detected on reader "); Serial.println(readerIndex + 1);
        
        // Check for ear test UID first (if ear disease)
        if (diseaseManager->requiresEarExamination() && 
            UIDManager::isEarTestUID(foundUID, foundSize)) {
            
            // Validate ear test is on correct reader (ears only)
            if (!UIDManager::isMedicineValidForReader(UIDManager::EARTEST, readerIndex)) {
                Serial.print("ERROR: Ear examination tool must be used on ear readers only");
                Serial.println();
                Serial.print("   Current location: ");
                Serial.println(UIDManager::getReaderLocationName(readerIndex));
                Serial.println("   INFO: Please use Right Ear (Reader 2) or Left Ear (Reader 4)");
                return;
            }
            
            Serial.println("Ear examination tool detected");
            handleEarTestDetection(readerIndex);
            changeState(STATE_EAR_TEST);
            return;
        }
        
        // Check for correct healing medicine
        if (UIDManager::isDiseaseHealingUID(foundUID, foundSize, diseaseManager->getCurrentDisease())) {
            UIDManager::MedicineType medicine = UIDManager::identifyMedicine(foundUID, foundSize);
            
            // Validate medicine is on correct reader location
            if (!UIDManager::isMedicineValidForReader(medicine, readerIndex)) {
                Serial.print("ERROR: ");
                Serial.print(UIDManager::getMedicineName(medicine));
                Serial.println(" cannot be used at this location!");
                Serial.print("   Current location: ");
                Serial.println(UIDManager::getReaderLocationName(readerIndex));
                
                // Provide helpful guidance based on medicine type
                switch (medicine) {
                    case UIDManager::AKAMOL:
                    case UIDManager::ANTIBIOTICS:
                        Serial.println("   INFO: This medicine must be placed in the Mouth (Reader 3)");
                        break;
                    case UIDManager::EARDROPS:
                        Serial.println("   INFO: Ear drops must be used on ear readers (Reader 2 or 4)");
                        break;
                    case UIDManager::OINTMENT:
                        Serial.println("   INFO: Ointment must be applied to body (Reader 1 or 5)");
                        break;
                    default:
                        break;
                }
                return;
            }
            
            if (!diseaseManager->isCorrectEarReader(readerIndex)) {
                Serial.println("ERROR: Wrong location for red ear - medicine must be applied to correct ear");
                return;
            }
            // store which medicine caused the healing transition
            lastMedicine = UIDManager::identifyMedicine(foundUID, foundSize);
            Serial.println("OK: Correct medicine detected - starting healing!");
            changeState(STATE_HEALING);
            return;
        }
        
        // Check for other medicine UIDs (wrong medicine but still react)
        UIDManager::MedicineType medicine = UIDManager::identifyMedicine(foundUID, foundSize);
        if (medicine != UIDManager::UNKNOWN) {
            
            // Validate medicine is on correct reader location
            if (!UIDManager::isMedicineValidForReader(medicine, readerIndex)) {
                Serial.print("ERROR: ");
                Serial.print(UIDManager::getMedicineName(medicine));
                Serial.println(" cannot be used at this location!");
                Serial.print("   Current location: ");
                Serial.println(UIDManager::getReaderLocationName(readerIndex));
                
                // Provide helpful guidance based on medicine type
                switch (medicine) {
                    case UIDManager::AKAMOL:
                    case UIDManager::ANTIBIOTICS:
                        Serial.println("   INFO: This medicine must be placed in the Mouth (Reader 3)");
                        break;
                    case UIDManager::EARDROPS:
                        Serial.println("   INFO: Ear drops must be used on ear readers (Reader 2 or 4)");
                        break;
                    case UIDManager::OINTMENT:
                        Serial.println("   INFO: Ointment must be applied to body (Reader 1 or 5)");
                        break;
                    default:
                        break;
                }
                return;
            }
            
            uint16_t track = UIDManager::getTrackForMedicine(medicine);
            hardware->playTrack(track);
            Serial.print("Doll reacting to "); Serial.print(UIDManager::getMedicineName(medicine));
            Serial.print(" (track "); Serial.print(track); Serial.println(")");
            Serial.print("   OK: Correct location: ");
            Serial.println(UIDManager::getReaderLocationName(readerIndex));
            changeState(STATE_REACTING_TO_MEDICINE);
            return;
        }
        
        Serial.println("Unknown medicine detected - no reaction");
    }
    // Note: When no medicine is present, doll continues crying (track 1 set in initialize)
}

void GameLogic::handleReactingToMedicineState() {
    byte foundUID[10];
    byte foundSize;
    int readerIndex;
    
    if (!checkForCard(foundUID, foundSize, readerIndex)) {
        // Medicine removed, doll goes back to being sick
        Serial.println("Medicine removed - doll becomes sick again");
        hardware->playTrack(1); // Resume crying sound
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
        Serial.println("Ear examination complete - doll is still sick");
        hardware->playTrack(1); // Resume crying sound
        changeState(STATE_SICK);
    }
    // NeoPixels remain on while examination is happening
}

void GameLogic::handleHealingState() {
    Serial.print("Healing "); Serial.print(diseaseManager->getCurrentDiseaseName());
    Serial.println(" with correct medicine!");
    hardware->turnOffAllLEDs();
    // uint16_t healingTrack = diseaseManager->getHealingTrack();
    // hardware->playTrack(healingTrack);
    // Serial.print("Playing happy/healing sound (track "); 
    // Serial.print(healingTrack); Serial.println(")");
    
    // stateTimer = millis();
    
    // // Wait for healing sound to finish
    // while (millis() - stateTimer < 5000) {
    //     delay(100);
    // }
    
    // hardware->stopAudio();
        // prefer the medicine track that caused the healing transition
    uint16_t playTrack = 0;
    if (lastMedicine != UIDManager::UNKNOWN) {
        playTrack = UIDManager::getTrackForMedicine(lastMedicine);
        Serial.print("Playing medicine track for ");
        Serial.print(UIDManager::getMedicineName(lastMedicine));
        Serial.print(" (track ");
        Serial.print(playTrack);
        Serial.println(")");
    } else {
        // fallback to disease-specific happy track
        playTrack = diseaseManager->getHealingTrack();
        Serial.print("Falling back to disease healing track (track ");
        Serial.print(playTrack);
        Serial.println(")");
    }
    
    hardware->playTrack(playTrack);
    
        // wait for the track (safety timeout)
    stateTimer = millis();
    const unsigned long maxWaitMs = 10000UL;
    const unsigned long pollInterval = 100UL;
    while (millis() - stateTimer < maxWaitMs) {
        delay(pollInterval);
            // optional: break early if mp3 status available
            // if (hardware->getMP3Player() && !hardware->getMP3Player()->isPlaying()) break;
    }

    // Skip waiting for healing sound and directly transition to GAME_OVER
    hardware->stopAudio();
    Serial.println("Doll is completely healed!");
    changeState(STATE_GAME_OVER);
}

void GameLogic::handleGameOverState() {
    // hardware->turnOffAllLEDs();
    Serial.println("Doll is healthy and happy! All symptoms gone - game complete!");
    Serial.println("Program ended.");
    
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
    Serial.print("State: ");
    Serial.print(stateNames[currentState]);
    Serial.print(" -> ");
    Serial.println(stateNames[newState]);
    
    currentState = newState;
    stateTimer = millis();
}