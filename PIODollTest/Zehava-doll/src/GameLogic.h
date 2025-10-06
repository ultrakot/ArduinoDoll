#pragma once
#include <Arduino.h>
#include "HardwareManager.h"
#include "DiseaseManager.h"
#include "UIDManager.h"

enum GameState {
    STATE_INIT,
    STATE_SICK,                    // doll is sick and crying
    STATE_REACTING_TO_MEDICINE,    // reacting to medicine
    STATE_EAR_TEST,
    STATE_HEALING,
    STATE_GAME_OVER
};

class GameLogic {
private:
    GameState currentState;
    unsigned long stateTimer;
    
    HardwareManager* hardware;
    DiseaseManager* diseaseManager;
    
public:
    GameLogic(HardwareManager* hw, DiseaseManager* dm);
    
    void initialize();
    void update();
    void changeState(GameState newState);
    
private:
    // State handlers
    void handleSickState();
    void handleReactingToMedicineState();
    void handleEarTestState();
    void handleHealingState();
    void handleGameOverState();
    
    // Helper functions
    bool checkForCard(byte* uid, byte& size, int& reader);
    void handleEarTestDetection(int readerIndex);
};