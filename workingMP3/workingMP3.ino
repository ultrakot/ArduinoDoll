#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

#define DF_RX_PIN  16   // ESP32 pin wired to DFPlayer TX
#define DF_TX_PIN  17   // ESP32 pin wired to DFPlayer RX

HardwareSerial dfSerial(2);       // use UART2
DFRobotDFPlayerMini player;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Explicitly map UART2 to your pins:
  dfSerial.begin(9600, SERIAL_8N1, DF_RX_PIN, DF_TX_PIN);
  Serial.printf("DFPlayer UART2 on RX=%d, TX=%d\n", DF_RX_PIN, DF_TX_PIN);

  Serial.println("Attempting DFPlayer.begin()...");
  if (!player.begin(dfSerial)) {
    Serial.println("▶️ player.begin() FAILED!");
    // Dump any raw bytes from DFPlayer to help you see what (if anything) it's sending:
    unsigned long deadline = millis() + 2000;
    while (millis() < deadline) {
      if (dfSerial.available()) {
        uint8_t b = dfSerial.read();
        Serial.printf("  0x%02X ", b);
      }
    }
    Serial.println("\n— end of dump —");
    Serial.println("Check wiring, voltage, and levels.");
    while (true) delay(0);
  }

  Serial.println("✅ DFPlayer initialized!");
  player.volume(20);
  player.play(2);
}

void loop() {
  // Nothing more needed here
}
