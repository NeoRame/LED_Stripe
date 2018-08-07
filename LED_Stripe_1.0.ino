// Use qsuba for smooth pixel colouring and qsubd for non-smooth pixel colouring
#define qsubd(x, b)  ((x>b)?b:0)                              // Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                            // Analog Unsigned subtraction macro. if result <0, then => 0

#include <FastLED.h>
#include <EEPROM.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

//This is where we adjust things to match our unique project:
#define NUM_LEDS    30          // Number of LEDs you have: 16 or more
#define LED_TYPE    WS2812B     // Type of LEDS. WS2812B 0r APA102. Don't forget to change the FastLED.addLeds line as well.
#define DATA_PIN    7           // Pin you've connected your LEDs to
#define BUTTON_A_PIN  2         // Connect the button to GND and one of the pins. (Use Interuppt Pin 2!!!)
#define BUTTON_B_PIN  3         // Connect the button to GND and one of the pins. (Use Interuppt Pin 3!!!)
//#define CLOCK_PIN   3           // Clock Pin you've connected your LEDs to
#define BRIGHTNESS  128         // 255 is full brightness, 128 is half, 32 is an eighth.
#define SATURATION  255         // 0-255, 0 is pure white, 255 is fully saturated color
#define COLOR_ORDER GRB         // Try mixing up the letters (RGB, GBR, BRG, etc) for a whole new world of color combinations. Use BGR for APA102 and GRB for WS2812

byte ledMode = 11;              // Update this number to the highest number of "cases"
uint8_t NUM_MODES = 11;         // Update this number to the highest number of "cases"
uint8_t gHue = 0;               // rotating "base color" used by many of the patterns

CRGB leds[NUM_LEDS];

CRGBPalette16 currentPalette;   // Palette definitions
CRGBPalette16 targetPalette;
TBlendType currentBlending = LINEARBLEND;


//------------------SETUP------------------
void setup() {
  //delay(100);                 // If things go bad, you can shutdown before the LED's start drawing power. (Default 3000)
  Serial.begin(57600);        // Get the serial port running for debugging

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);    // Use this for WS2812B
  //FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);                      // Use this for WS2801 or APA102
  digitalWrite (BUTTON_A_PIN, HIGH);                                                                  // internal pull-up resistor
  attachInterrupt (digitalPinToInterrupt (BUTTON_A_PIN), changeEffect, CHANGE);                       // pressed
  digitalWrite (BUTTON_B_PIN, HIGH);                                                                  // internal pull-up resistor
  attachInterrupt (digitalPinToInterrupt (BUTTON_B_PIN), changeEffect, CHANGE);                       // pressed

  FastLED.setBrightness(BRIGHTNESS);
  currentBlending;
  currentPalette = OceanColors_p;

  ledMode = EEPROM.read(0);

  if (ledMode > NUM_MODES) ledMode = 0;   // A safety in case the EEPROM has an illegal value.
  
  Serial.print("Starting LED Mode: ");
  Serial.println(ledMode);
}


//------------------MAIN LOOP------------------
void loop() {

  EVERY_N_MILLISECONDS(100) {
    uint8_t maxChanges = 24;
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
  }

  EVERY_N_SECONDS(5) {                                                       // Change the target palette to a random one every 5 seconds.
    uint8_t baseC = random8();                                               // You can use this as a baseline colour if you want similar hues in the next line.
    targetPalette = CRGBPalette16(CHSV(baseC + random8(32), 192, random8(128, 255)), CHSV(baseC + random8(32), 255, random8(128, 255)), CHSV(baseC + random8(32), 192, random8(128, 255)), CHSV(baseC + random8(32), 255, random8(128, 255)));
  }

  switch (ledMode) {
    case 0:  TARDISPulse(255, 255, 255);  break;                   // R,G,B
    case 1:  CylonBounce(255, 255, 255, 5);  break;                // R,G,B, EySize
    case 2:  KITT(); break;
    case 3:  threeDots();  break;
    case 4:  Plasma();  break;
    case 5:  Sinelon(); break;
    case 6:  Rainbow(); break;
    case 7:  RainbowWithGlitter();  break;
    case 8:  Sparkle(255, 255, 255);  break;                       // R,G,B
    case 9:  MeteorRain( 153, 153, 250, 5, 96, true, 45);  break;  // R,G,B, Meteor size, meteor tail decays/ disappears, meteorRandomDecay, SpeedDelay
    case 10: Fire(55, 120, 10);  break;                            // Cooling, Sparking, SpeedDelay
    case 11: PoliceScanner(3, 6, 60);  break;                      // EffectCount, StrobeCount, FlashDelay
  }

  // Power managed display
  set_max_power_in_volts_and_milliamps(5, 1000);                   // This is defined in setup and used by the power management functionality and is currently set at 5V, 500mA.
  show_at_max_brightness_for_power();                              // This is used in loop for power managed display of LED's.

  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
}


//------------------BUTTON PRESS SETUP------------------
void changeEffect() {
  if (digitalRead (BUTTON_A_PIN) == HIGH) {
    ledMode++;
    EEPROM.put(0, ledMode);
    asm volatile ("  jmp 0");
  }

  if (digitalRead (BUTTON_B_PIN) == HIGH) {
    ledMode--;
    //ledMode = 11;                           //Police Lights
    EEPROM.put(0, ledMode);
    asm volatile ("  jmp 0");
  }
}


//------------------LED EFFECTS------------------
// White Pulsing Light, like the TARDIS Lantern
void TARDISPulse(byte red, byte green, byte blue) {
  float r, g, b;

  for (int k = 0; k < 256; k = k + 1) {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setAll(r, g, b);
    FastLED.show();
  }

  for (int k = 255; k >= 0; k = k - 2) {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setAll(r, g, b);
    FastLED.show();
  }
}

// White Knight Rider Scanner Light
void CylonBounce(byte red, byte green, byte blue, int EyeSize) {

  for (int i = 0; i < NUM_LEDS - EyeSize - 2; i++) {
    setAll(0, 0, 0);
    setPixel(i, red / 10, green / 10, blue / 7);
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, red, green, blue);
    }
    setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 7);
    FastLED.show();
    delay(35);
  }

  for (int i = NUM_LEDS - EyeSize - 2; i > 0; i--) {
    setAll(0, 0, 0);
    setPixel(i, red / 10, green / 10, blue / 7);
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, red, green, blue);
    }
    setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 7);
    FastLED.show();
    delay(35);
  }

}

// Knight Rider Scanner from KITT
void KITT() {
  fadeToBlackBy( leds, NUM_LEDS, 12);
  int pos = beatsin16( 24, 0, NUM_LEDS - 1 );
  leds[pos] += CRGB::Red;
  delay(3);
}

// Three Moving Dots
void threeDots() {
  uint8_t inner = beatsin8(30, NUM_LEDS / 4 - 1, NUM_LEDS / 4 + 16);  // Move 1/4 to 3/4
  uint8_t outer = beatsin8(30, 0, NUM_LEDS - 1);                      // Move entire length
  uint8_t middle = beatsin8(30, NUM_LEDS / 3 - 1, NUM_LEDS / 3 * 2);  // Move 1/3 to 2/3

  leds[middle] = CRGB::Purple;
  leds[inner] = CRGB::Blue;
  leds[outer] = CRGB::Aqua;

  nscale8(leds, NUM_LEDS, 224);                                       // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);

}

// Plasma Effect
void Plasma() {

  int thisPhase = beatsin8(6, -64, 64);                               // Setting phase change for a couple of waves.
  int thatPhase = beatsin8(7, -64, 64);

  for (int k = 0; k < NUM_LEDS; k++) {                                // For each of the LED's in the strand, set a brightness based on a wave as follows:

    int colorIndex = cubicwave8((k * 23) + thisPhase) / 2 + cos8((k * 15) + thatPhase) / 2; // Create a wave and add a phase change and add another wave with its own phase change.. Hey, you can even change the frequencies if you wish.
    int thisBright = qsuba(colorIndex, beatsin8(7, 0, 96));                                 // qsub gives it a bit of 'black' dead space by setting sets a minimum value. If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex..

    leds[k] = ColorFromPalette(currentPalette, colorIndex, thisBright, currentBlending);    // Let's now add the foreground colour.
  }

}

// A cool colorful ripple effect
void Sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
  delay(30);
}

// Rainbow Effect
void Rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

// Glitter Effect for "RainbowWithGlitter"
void addGlitter( fract8 chanceOfGlitter) {
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;

    delay(30);
  }
}

// Rainbow Effect with Glitter
void RainbowWithGlitter() {
  // built-in FastLED rainbow, plus some random sparkly glitter
  Rainbow();
  addGlitter(80);
}

// Nice Sparkle Effect
void Sparkle(byte red, byte green, byte blue) {
  fadeToBlackBy( leds, NUM_LEDS, 250);
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel, red, green, blue);
  FastLED.show();
  delay(30);
  setPixel(Pixel, 0, 0, 0);
}

// Like a falling Meteor
void MeteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {
  setAll(0, 0, 0);

  for (int i = 0; i < NUM_LEDS + NUM_LEDS; i++) {

    // fade brightness all LEDs one step
    for (int j = 0; j < NUM_LEDS; j++) {
      if ( (!meteorRandomDecay) || (random(10) > 5) ) {
        fadeToBlack(j, meteorTrailDecay );
      }
    }

    // draw Meteor
    for (int j = 0; j < meteorSize; j++) {
      if ( ( i - j < NUM_LEDS) && (i - j >= 0) ) {
        setPixel(i - j, red, green, blue);
      }
    }
    FastLED.show();
    delay(SpeedDelay);
  }
}

// LEDs on Fire
void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUM_LEDS];
  int cooldown;

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);

    if (cooldown > heat[i]) {
      heat[i] = 0;
    } else {
      heat[i] = heat[i] - cooldown;
    }
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if ( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160, 255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }
  FastLED.show();
  delay(SpeedDelay);
}

void setPixelHeatColor (int Pixel, byte temperature) {

  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature / 255.0) * 191);

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if ( t192 > 0x80) {                    // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if ( t192 > 0x40 ) {            // middle
    setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}

// Police Scanner Lights
void PoliceScanner(int EffectCount, int StrobeCount, int FlashDelay) {
  for (int j = 0; j < EffectCount; j++) {
    for (int j = 0; j < StrobeCount; j++) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      fill_solid(leds, NUM_LEDS / 2, CRGB::White);
      fill_solid(leds, NUM_LEDS / 4 + 5, CRGB::Red);
      FastLED.show();
      delay(FlashDelay);
      setAll(0, 0, 0);
      FastLED.show();
      delay(FlashDelay);
    }
    for (int j = 0; j < 6; j++) {
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      fill_solid(leds, NUM_LEDS / 2 + 3, CRGB::White);
      fill_solid(leds, NUM_LEDS / 2, CRGB::Black);
      FastLED.show();
      delay(FlashDelay);
      setAll(0, 0, 0);
      FastLED.show();
      delay(FlashDelay);
    }
  }
  for (int j = 0; j < EffectCount; j++) {
    for (int j = 0; j < StrobeCount; j++) {
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      fill_solid(leds, NUM_LEDS / 4 + 17, CRGB::Black);
      fill_solid(leds, NUM_LEDS / 2 + 3, CRGB::White);
      fill_solid(leds, NUM_LEDS / 4 + 5, CRGB::Black);
      fill_solid(leds, NUM_LEDS / 4 - 1, CRGB::Red);
      FastLED.show();
      delay(FlashDelay);
      setAll(0, 0, 0);
      fill_solid(leds, NUM_LEDS / 2 + 3, CRGB::White);
      fill_solid(leds, NUM_LEDS / 4 + 5, CRGB::Black);
      FastLED.show();
      delay(FlashDelay);
    }
    for (int j = 0; j < 6; j++) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      fill_solid(leds, NUM_LEDS / 4 + 17, CRGB::Blue);
      fill_solid(leds, NUM_LEDS / 2 + 3, CRGB::White);
      fill_solid(leds, NUM_LEDS / 4 + 5, CRGB::Red);
      fill_solid(leds, NUM_LEDS / 4 - 1, CRGB::Black);
      FastLED.show();
      delay(FlashDelay);
      setAll(0, 0, 0);
      fill_solid(leds, NUM_LEDS / 2 + 3, CRGB::White);
      fill_solid(leds, NUM_LEDS / 4 + 5, CRGB::Black);
      FastLED.show();
      delay(FlashDelay);
    }
  }
}


//------------------MISC SETTINGS------------------
void fadeToBlack(int ledNo, byte fadeValue) {
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  leds[ledNo].fadeToBlackBy( fadeValue );
#endif
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
#endif
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}
