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
    // Initialize DFPlayer
    Serial.println("MP3: calling _player.begin()...");
    if (!_player.begin(*_serial)) {
        Serial.println("▶️ DFPlayer.begin() failed");
        _ready = false;
        return false;
    }

    Serial.println("MP3: DFPlayer.begin() succeeded");
    Serial.println("MP3: setting volume to 20");
    _player.volume(20);
    Serial.println("✅ DFPlayer initialized and volume set");
    _ready = true;
    return true;
}

void MP3Player::playTrack(uint16_t trackNum) {
    Serial.printf("MP3: playTrack(%d) requested\n", trackNum);
    Serial.printf("MP3: sending play command to DFPlayer (track %d)\n", trackNum);
    _player.play(trackNum);
    Serial.printf("MP3: play command sent for track %d\n", trackNum);
}

void MP3Player::stop() {
    Serial.println("MP3: stop() requested");
    Serial.println("MP3: sending stop command to DFPlayer...");
    _player.stop();
    Serial.println("MP3: stop command sent");
}

bool MP3Player::isReady() const {
    return _ready;
}
