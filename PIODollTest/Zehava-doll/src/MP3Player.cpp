#include "MP3Player.h"

MP3Player::MP3Player(uint8_t rxPin, uint8_t txPin, uint8_t uartNum)
    : _rxPin(rxPin), _txPin(txPin), _uartNum(uartNum), _serial(nullptr), _ready(false) {}

bool MP3Player::begin() {
    if (_uartNum == 2) {
        _serial = &Serial2;
    } else if (_uartNum == 1) {
        _serial = &Serial1;
    } else {
        _serial = &Serial;
    }
    _serial->begin(9600, SERIAL_8N1, _rxPin, _txPin);
    Serial.printf("DFPlayer UART%d on RX=%d, TX=%d\n", _uartNum, _rxPin, _txPin);
    Serial.println("Attempting DFPlayer.begin()...");
    if (!_player.begin(*_serial)) {
        Serial.println("▶️ player.begin() FAILED!");
        unsigned long deadline = millis() + 2000;
        while (millis() < deadline) {
            if (_serial->available()) {
                uint8_t b = _serial->read();
                Serial.printf("  0x%02X ", b);
            }
        }
        Serial.println("\n— end of dump —");
        Serial.println("Check wiring, voltage, and levels.");
        _ready = false;
        return false;
    }
    Serial.println("✅ DFPlayer initialized!");
    _player.volume(20); // Increase volume slightly
    delay(100); // Give time for volume command
    _ready = true;
    return true;
}

void MP3Player::playTrack(uint16_t trackNum) {
    if (_ready) {
        _player.play(trackNum);
        delay(50); // Small delay to ensure command is processed
    }
}

void MP3Player::stop() {
    if (_ready) {
        _player.stop();
    }
}

bool MP3Player::isReady() const {
    return _ready;
}
