#include <IRremote.hpp>

#define IR_RECEIVE_PIN 26  // change this to the pin you connected your IR receiver to

void setup() {
  Serial.begin(115200);
  // Start the IR receiver: pin, and enable LED feedback (optional)
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println(F("IR Receiver ready"));
}

void loop() {
  if (IrReceiver.decode()) {
    // Print the raw data (in hexadecimal)
    Serial.print(F("Raw Data: 0x"));
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    // Print a short result: protocol, address, command
    IrReceiver.printIRResultShort(&Serial);

    // Print how you could send it (for debugging / reuse)
    IrReceiver.printIRSendUsage(&Serial);

    // Allow the receiver to get the next value
    IrReceiver.resume();
  }
}
