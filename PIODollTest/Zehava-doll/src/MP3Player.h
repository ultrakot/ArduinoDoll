#pragma once
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>
#include <Arduino.h>

class MP3Player {
public:
    MP3Player(uint8_t rxPin, uint8_t txPin, uint8_t uartNum = 2);
    bool begin();
    void playTrack(uint16_t trackNum);
    void stop();
    bool isReady() const;
private:
    HardwareSerial *_serial;
    DFRobotDFPlayerMini _player;
    uint8_t _rxPin, _txPin, _uartNum;
    bool _ready;
};
