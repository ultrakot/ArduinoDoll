#pragma once
#include <Arduino.h>
#include "NFCVSPI.h"
#include "MP3Player.h"
#include "IRController.h"
#include <Adafruit_NeoPixel.h>

class HardwareManager {
private:
    NFCVSPI* nfcReader;
    MP3Player* mp3Player;
    IRController* irController;
    Adafruit_NeoPixel* npRubAll;
    Adafruit_NeoPixel* npRubAll2;
    Adafruit_NeoPixel* npEar;
    Adafruit_NeoPixel* npEar2;
    
    uint16_t currentTrack;
    
    // Button on pin 34
    static const uint8_t BUTTON_PIN = 34;
    bool buttonFlag;
    bool lastButtonState;
    unsigned long lastDebounceTime;
    static const unsigned long DEBOUNCE_DELAY = 50;

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
    void updateIR(); // Update IR controller
    void updateButton(); // Update button state
    bool getButtonFlag() const { return buttonFlag; }
    uint16_t getCurrentTrack() const { return currentTrack; }
    
    // Direct access for specific cases
    NFCVSPI* getNFCReader() { return nfcReader; }
    MP3Player* getMP3Player() { return mp3Player; }
};