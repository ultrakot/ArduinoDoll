#include "GameLogic.h"
#include "HardwareManager.h"
#include "DiseaseManager.h"
#include "UIDManager.h"

// Global game components
HardwareManager hardware;
DiseaseManager diseaseManager;
GameLogic gameLogic(&hardware, &diseaseManager);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("🎮 Zehava Doll Starting...");
    
    // Initialize all systems
    Serial.println("🚀 Initializing game...");
    hardware.initialize();
    diseaseManager.initializeFromEEPROM();
    gameLogic.initialize();
    
    Serial.println("🎯 Game ready!");
    
    // Force audio start as backup
    Serial.println("🔊 Starting crying sound...");
    Serial.println("😷 (TEST MODE) Audio disabled - would start crying sound (track 1)");
}

void loop() {
    gameLogic.update();
    delay(100);
}
