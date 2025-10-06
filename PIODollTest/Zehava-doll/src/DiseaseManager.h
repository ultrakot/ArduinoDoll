#pragma once
#include <Arduino.h>
#include <EEPROM.h>

class DiseaseManager {
private:
    struct Disease {
        const char* name;
        uint16_t healingTrack;
        bool isEarDisease;
    };
    
    static const Disease diseases[];
    static const int numDiseases;
    static const int EEPROM_ADDR;
    
    int currentDisease;
    int led22Color; // For ear diseases: 0=healthy, 1=infected

public:
    DiseaseManager();
    
    void initializeFromEEPROM();
    int getCurrentDisease() const { return currentDisease; }
    const char* getCurrentDiseaseName() const;
    uint16_t getHealingTrack() const;
    bool isEarDisease() const;
    bool requiresEarExamination() const { return isEarDisease(); }
    
    // Ear-specific functions
    int getEarColor() const { return led22Color; }
    void generateRandomEarColor();
    bool isCorrectEarReader(int readerIndex) const;
    
    // Disease count
    static int getTotalDiseases() { return numDiseases; }
};