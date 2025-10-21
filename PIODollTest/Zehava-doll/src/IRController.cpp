#include "IRController.h"
#include <IRremote.hpp>  // Include ONLY in .cpp file to avoid linker errors

IRController::IRController(uint8_t sendPin)
    : _sendPin(sendPin), _active(false), _lastSendTime(0), 
      _sendInterval(1000), _command(0x34) {}

bool IRController::begin() {
    Serial.print(F("Initializing IR sender on pin "));
    Serial.println(_sendPin);
    
    IrSender.begin(_sendPin, ENABLE_LED_FEEDBACK);
    
    Serial.println(F("IR sender initialized"));
    return true;
}

void IRController::start(uint8_t command) {
    if (!_active) {
        _command = command;
        _active = true;
        _lastSendTime = 0;  // Send immediately on next update()
        Serial.print(F("IR signal started - command: 0x"));
        Serial.println(_command, HEX);
    }
}

void IRController::stop() {
    if (_active) {
        _active = false;
        Serial.println(F("IR signal stopped"));
    }
}

void IRController::update() {
    if (_active) {
        unsigned long currentTime = millis();
        
        // Send IR signal at specified interval
        if (currentTime - _lastSendTime >= _sendInterval) {
            // Send NEC protocol: address=0x00, command=_command, repeats=0
            IrSender.sendNEC(0x00, _command, 0);
            _lastSendTime = currentTime;
            
            // Optional: print periodic status
            static uint8_t sendCount = 0;
            if (++sendCount % 10 == 0) {  // Print every 10 sends
                Serial.print(F("IR signal sent ("));
                Serial.print(sendCount);
                Serial.println(F(" times)"));
            }
        }
    }
}

bool IRController::isActive() const {
    return _active;
}

void IRController::setInterval(uint16_t intervalMs) {
    _sendInterval = intervalMs;
    Serial.print(F("IR interval set to "));
    Serial.print(intervalMs);
    Serial.println(F(" ms"));
}

void IRController::setCommand(uint8_t command) {
    _command = command;
    Serial.print(F("IR command set to 0x"));
    Serial.println(command, HEX);
}
