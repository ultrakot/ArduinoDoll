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
    Serial.println("Zehava Doll Starting...");
    
    // Initialize all systems
    Serial.println("Initializing game...");
    hardware.initialize();
    diseaseManager.initializeFromEEPROM();
    gameLogic.initialize();
    
    // Uncomment line below for LED testing
    // hardware.testAllLEDs();
    
    Serial.println("Game ready!");
}

void loop() {
    gameLogic.update();
    hardware.updateIR(); // Update IR controller
    hardware.updateButton(); // Update button state
    delay(100);
}
