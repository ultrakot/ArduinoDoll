#include "DiseaseManager.h"

const DiseaseManager::Disease DiseaseManager::diseases[] = {
    {"Rubella", 5, false},
    {"Allergy", 4, false},
    {"Ear Pain", 2, true},
    {"Ear Infection", 2, true},
    {"Fever", 3, false}
};

const int DiseaseManager::numDiseases = sizeof(diseases) / sizeof(diseases[0]);
const int DiseaseManager::EEPROM_ADDR = 0;

DiseaseManager::DiseaseManager() : currentDisease(-1), led22Color(0) {
}

void DiseaseManager::initializeFromEEPROM() {
    EEPROM.begin(4);
    int diseaseIndex = EEPROM.read(EEPROM_ADDR);
    if (diseaseIndex < 0 || diseaseIndex >= numDiseases) diseaseIndex = 0;
    currentDisease = diseaseIndex;
    
    // Update EEPROM for next run
    diseaseIndex++;
    if (diseaseIndex >= numDiseases) diseaseIndex = 0;
    EEPROM.write(EEPROM_ADDR, diseaseIndex);
    EEPROM.commit();
    
    // Generate random ear color for ear diseases
    if (isEarDisease()) {
        generateRandomEarColor();
    }
    
    Serial.print("🏥 Active disease: "); Serial.println(getCurrentDiseaseName());
}

const char* DiseaseManager::getCurrentDiseaseName() const {
    if (currentDisease >= 0 && currentDisease < numDiseases) {
        return diseases[currentDisease].name;
    }
    return "Unknown";
}

uint16_t DiseaseManager::getHealingTrack() const {
    if (currentDisease >= 0 && currentDisease < numDiseases) {
        return diseases[currentDisease].healingTrack;
    }
    return 1; // Default to crying track
}

bool DiseaseManager::isEarDisease() const {
    if (currentDisease >= 0 && currentDisease < numDiseases) {
        return diseases[currentDisease].isEarDisease;
    }
    return false;
}

void DiseaseManager::generateRandomEarColor() {
    randomSeed(millis());
    led22Color = random(2); // 0=healthy, 1=infected
}

bool DiseaseManager::isCorrectEarReader(int readerIndex) const {
    // For ear diseases with red LED (infected), only reader2 (index 1) can heal
    if (isEarDisease() && led22Color == 1) {
        return readerIndex == 1;
    }
    return true; // For non-ear diseases or healthy ear, any reader is fine
}