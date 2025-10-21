#ifndef IRCONTROLLER_H
#define IRCONTROLLER_H

#include <Arduino.h>
// Forward declare - do NOT include IRremote.hpp in header to avoid multiple definition errors

class IRController {
private:
    uint8_t _sendPin;
    bool _active;
    unsigned long _lastSendTime;
    uint16_t _sendInterval;  // Interval between IR transmissions (ms)
    uint8_t _command;        // IR command to send
    
public:
    IRController(uint8_t sendPin = 17);
    
    bool begin();
    void start(uint8_t command = 0x34);  // Start sending IR signal
    void stop();                          // Stop sending IR signal
    void update();                        // Call in loop to maintain signal
    bool isActive() const;
    void setInterval(uint16_t intervalMs);
    void setCommand(uint8_t command);
};

#endif
