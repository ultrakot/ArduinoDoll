// /*  
//   Turns on an Electromagnet for one second, then off for one second, repeatedly.  
//   This example code is in the public domain.  
// */

// int Electromagnet = 26;    // D2 on Grove Base Shield  
// int LED           = 32;   // onboard LED  

// void setup() {
//     pinMode(Electromagnet, OUTPUT);
//     pinMode(LED, OUTPUT);
// }

// void loop() {
//     digitalWrite(Electromagnet, HIGH);  // Electromagnet on  
//     digitalWrite(LED,           HIGH);  // LED on  
//     delay(1000);                        // wait 1 s  

//     digitalWrite(Electromagnet, LOW);   // Electromagnet off  
//     digitalWrite(LED,           LOW);   // LED off  
//     delay(1000);                        // wait 1 s  
// }

#include <Adafruit_NeoPixel.h>

// Pin definitions
const uint8_t MAG_PIN   = 26;   // D2 on Grove Base Shield
const uint8_t PIXEL_PIN = 32;   // Any PWM-capable pin wired to your NeoPixel’s DIN
const uint8_t PIXEL_COUNT = 1;

// NeoPixel setup
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(MAG_PIN, OUTPUT);
  strip.begin();    // Initialize NeoPixel library :contentReference[oaicite:0]{index=0}
  strip.show();     // Ensure all pixels are off :contentReference[oaicite:1]{index=1}
}

void loop() {
  // --- Turn electromagnet (and pixel) ON ---
  digitalWrite(MAG_PIN, HIGH);
  strip.setPixelColor(0, strip.Color(255, 255, 255));  // full-white :contentReference[oaicite:2]{index=2}
  strip.show();                                        // push update :contentReference[oaicite:3]{index=3}
  delay(1000);

  // --- Turn electromagnet (and pixel) OFF ---
  digitalWrite(MAG_PIN, LOW);
  strip.clear();   // set pixel to “off” :contentReference[oaicite:4]{index=4}
  strip.show();
  delay(1000);
}
