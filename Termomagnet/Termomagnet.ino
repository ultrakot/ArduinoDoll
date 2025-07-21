#include <Adafruit_NeoPixel.h>

// ←– Adjust these to match your wiring ↓
#define REED_PIN    2     // reed switch: closes to GND when magnet is present
#define BUTTON_PIN  22     // push-button: one side to GND, the other to this pin
#define LED_PIN     21     // NeoPixel DIN
#define NUM_PIXELS 14     // length of your strip

Adafruit_NeoPixel strip(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  pinMode(REED_PIN,   INPUT_PULLUP);   // reed pulls LOW when closed
  pinMode(BUTTON_PIN, INPUT_PULLUP);   // button to GND
  strip.begin();
  strip.show();                        // ensure all pixels start OFF
}

void loop() {
  // if button is pressed (reads LOW)…
  if (digitalRead(BUTTON_PIN) == LOW) {
    // 1) Read reed switch
    bool magnetPresent = (digitalRead(REED_PIN) == LOW);

    // 2) Print its state once
    Serial.print("Reed switch: ");
    Serial.println(magnetPresent ? "CLOSED (magnet present)" : "OPEN (no magnet)");

    // 3) Light the strip RED or GREEN
    uint32_t color = magnetPresent
      ? strip.Color(255, 0, 0)   // red
      : strip.Color(0, 255, 0);  // green

    for (int i = 0; i < NUM_PIXELS; i++) {
      strip.setPixelColor(i, color);
    }
    strip.show();

    // 4) Hold here (strip stays lit) until the button is released
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10);
    }

    // 5) Button released → turn everything off
    strip.clear();
    strip.show();
  }
}
