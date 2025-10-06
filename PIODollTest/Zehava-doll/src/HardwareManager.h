#pragma once
#include <Arduino.h>
#include "NFCVSPI.h"
#include "MP3Player.h"
#include <Adafruit_NeoPixel.h>

class HardwareManager {
private:
    NFCVSPI* nfcReader;
    MP3Player* mp3Player;
    Adafruit_NeoPixel* npRubAll;
    Adafruit_NeoPixel* npRubAll2;
    Adafruit_NeoPixel* npEar;
    Adafruit_NeoPixel* npEar2;
    
    uint16_t currentTrack;

public:
    HardwareManager();
    ~HardwareManager();
    
    void initialize();
    bool detectCard(byte* uid, byte& size, int& reader);
    void playTrack(uint16_t track);
    void stopAudio();
    void setDiseaseSymptoms(int disease, int earColor = 0);
    void setEarExaminationResult(int reader, bool infected);
    void turnOffAllLEDs();
    void testAllLEDs(); // Add test function
    uint16_t getCurrentTrack() const { return currentTrack; }
    
    // Direct access for specific cases
    NFCVSPI* getNFCReader() { return nfcReader; }
    MP3Player* getMP3Player() { return mp3Player; }
};