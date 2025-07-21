#include <Adafruit_NeoPixel.h>

//––– Configuration
#define NUM_PIXELS 4
uint8_t LED_PINS[NUM_PIXELS] = { 33, 32, 22, 21 };  
//––– Create one NeoPixel instance per pin (each has exactly 1 LED)
Adafruit_NeoPixel pixels[NUM_PIXELS] = {
  Adafruit_NeoPixel(1, LED_PINS[0], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(1, LED_PINS[1], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(1, LED_PINS[2], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(1, LED_PINS[3], NEO_GRB + NEO_KHZ800),
};

void setup() {
  // Initialize each NeoPixel
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels[i].begin();
    pixels[i].show();  // make sure it starts “off”
  }
}

void loop() {
  // 1) Scan a single red pixel from first to last
  for (int i = 0; i < NUM_PIXELS; i++) {
    // clear all LEDs
    for (int j = 0; j < NUM_PIXELS; j++) {
      pixels[j].clear();
      pixels[j].show();
    }
    // light only pixel i in red
    pixels[i].setPixelColor(0, pixels[i].Color(255, 0, 0));
    pixels[i].show();
    delay(500);  // how long each stays lit
  }

  // 2) Hold all off for 2 seconds
  for (int j = 0; j < NUM_PIXELS; j++) {
    pixels[j].clear();
    pixels[j].show();
  }
  delay(2000);

  // 3) Short pause before repeating
  delay(200);
}
