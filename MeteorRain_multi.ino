#include <Adafruit_NeoPixel.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
typedef struct PixelStrip {
  Adafruit_NeoPixel strip;
  byte red;
  byte green;
  byte blue;
  byte meteorsize;
  byte meteortraildecay;
  bool meteorrandomdecay;
  byte speeddelay;
  byte currled;
  byte numleds;
  int countdown;
  bool endrandom;
  int16_t enddelay;
  int16_t randomenddelaystart;
  int16_t randomenddelayend;
  bool beginrandom;
  int16_t randombegindelaystart;
  int16_t randombegindelayend;
  byte bgred;
  byte bggreen;
  byte bgblue;
} PixelStrip;

/*************************************************************
strip: defines a new LED strip, needs the following parameters: number of LEDs and PIN, leave the NEO_GRB + NEO_KHZ800 as is
red, green, blue: RGB values for strip colour
meteorsize: number of LEDS that the bright meteor will be displayed as
meteortraildecay: how fast the trail fades to black
meteorrandomdecay: if set to true, meteor tail decays at random spots else it will decay linearly.
speeddelay: the speed of the meteor
currled: leave at 0, used internally.
numleds: should be exactly like the number of LEDs defined in the strip.
countdown: leave at 0, used internally.
endrandom: if set to true, the next meteor will take a random number between randomenddelaystart and randomenddelayend to start
enddelay: how long it takes to start a new meteor after one finishes.
randomenddelaystart: minimum time taken to start a new meteor
randomenddelayend: maximum time taken to start a new meteor
beginrandom: determines if the first meteor after being switched on will be delayed with a random number between randombegindelaystart and randombegindelayend
randombegindelaystart: minimum time taken to start first meteor
randombegindelayend: maximum time taken to start first meteor
bgred, bggreen, bgblue: background colours for LED strip
*************************************************************/

PixelStrip strips[] = {
  // strip            leds  pin                           red   green blue  meteorsize  meteortraildecay  meteorrandomdecay speeddelay  currled(0)  numleds countdown(0)  endrandom   enddelay  randomenddelaystart   randomenddelayend   beginrandom   randombegindelaystart   randombegindelayend   bgred   bggreen   bgblue
  { Adafruit_NeoPixel(40, 6, NEO_GRB + NEO_KHZ800), 10, 255, 10, 1, 64, true, 50, 0, 40, 0, false, 500, 0, 500, false, 0, 5000, 2, 2, 2 },
  { Adafruit_NeoPixel(40, 12, NEO_GRB + NEO_KHZ800), 210, 120, 50, 1, 64, true, 20, 0, 40, 0, false, 20, 0, 500, false, 0, 5000, 2, 2, 2 },
  { Adafruit_NeoPixel(40, 7, NEO_GRB + NEO_KHZ800), 255, 255, 10, 1, 64, true, 60, 0, 40, 0, true, 20, 0, 500, true, 0, 5000, 10, 10, 10 },
  { Adafruit_NeoPixel(40, 8, NEO_GRB + NEO_KHZ800), 255, 10, 10, 1, 64, true, 60, 0, 40, 0, true, 20, 0, 500, true, 0, 5000, 10, 10, 10 },
  { Adafruit_NeoPixel(40, 9, NEO_GRB + NEO_KHZ800), 10, 10, 255, 1, 64, true, 60, 0, 40, 0, true, 20, 0, 500, true, 0, 5000, 10, 10, 10 },
  { Adafruit_NeoPixel(40, 10, NEO_GRB + NEO_KHZ800), 10, 10, 255, 1, 64, true, 60, 0, 40, 0, true, 20, 0, 500, true, 0, 5000, 10, 10, 10 },
  { Adafruit_NeoPixel(40, 11, NEO_GRB + NEO_KHZ800), 10, 10, 255, 1, 64, true, 60, 0, 40, 0, true, 20, 0, 500, true, 0, 5000, 10, 10, 10 },
  { Adafruit_NeoPixel(40, 13, NEO_GRB + NEO_KHZ800), 10, 255, 10, 1, 64, true, 50, 0, 40, 0, true, 500, 0, 500, true, 0, 5000, 10, 10, 10 },

};

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

#define NUMSTRIPS (sizeof(strips) / sizeof(strips[0]))

// speed for screen wipe at the beginning
#define WIPESPEED 50


void setup() {
  Serial.begin(115200);
  byte max_numleds = 0;

  for (int i = 0; i < NUMSTRIPS; i++) {
    if (strips[i].numleds > max_numleds) max_numleds = strips[i].numleds;
  }
  
  for (int i = 0; i < NUMSTRIPS; i++) {
    strips[i].strip.begin();
    strips[i].strip.show();  // Initialize all pixels to 'off'
    setAll(&(strips[i]), 0, 0, 0);
    
  }

  //perform screen wipe
  for (int i = 0; i < max_numleds; i++) {
    screenWipe();
    delay(WIPESPEED);
  }

  //reset led counter 
  for (int i = 0; i < NUMSTRIPS; i++) {
    strips[i].currled = 0;
  }
  delay(1000);
}

void screenWipe() {
  for (int k = 0; k < NUMSTRIPS; k++) {

    // end of meteor animation
    if (strips[k].currled > strips[k].numleds) {

      continue;
    }

    // draw screen
    if ((strips[k].currled <= strips[k].numleds) && (strips[k].currled >= 0)) {
      setPixel(&strips[k].strip, strips[k].currled, strips[k].bgred, strips[k].bggreen, strips[k].bgblue);
    }

    
    showStrip(&(strips[k].strip));
    strips[k].currled++;
  }
}

void loop() {
  //Serial.println(sizeof(strips));
  //Serial.println(sizeof(strips[0]));
  meteorRain();
  delay(1);
}

void fadeToColor(Adafruit_NeoPixel *strip, int ledNo, byte fadeValue, byte bgred, byte bggreen, byte bgblue) {

  int32_t oldColor;
  int8_t r, g, b;
  int value;

  oldColor = strip->getPixelColor(ledNo);
  r = (oldColor & 0x00ff0000UL) >> 16;
  g = (oldColor & 0x0000ff00UL) >> 8;
  b = (oldColor & 0x000000ffUL);



  r = (r <= bgred + 10) ? bgred : (int)r - (r * fadeValue / 256);
  g = (g <= bggreen + 10) ? bggreen : (int)g - (g * fadeValue / 256);
  b = (b <= bgblue + 10) ? bgblue : (int)b - (b * fadeValue / 256);

  strip->setPixelColor(ledNo, r, g, b);
}

void showStrip(Adafruit_NeoPixel *strip) {
  strip->show();
}

void gainToColor(Adafruit_NeoPixel *strip, int ledNo, byte fadeValue, int8_t led_r, int8_t led_g, int8_t led_b) {

  int32_t oldColor;
  int8_t r, g, b;
  int value;

  oldColor = strip->getPixelColor(ledNo);
  r = (oldColor & 0x00ff0000UL) >> 16;
  g = (oldColor & 0x0000ff00UL) >> 8;
  b = (oldColor & 0x000000ffUL);

  r = (r >= led_r - 10) ? led_r : (int)r + (r * fadeValue / 256);
  g = (g >= led_g - 10) ? led_g : (int)g + (g * fadeValue / 256);
  b = (b >= led_b - 10) ? led_b : (int)b + (b * fadeValue / 256);

  strip->setPixelColor(ledNo, r, g, b);
}
void setPixel(Adafruit_NeoPixel *strip, int Pixel, byte red, byte green, byte blue) {
  strip->setPixelColor(Pixel, strip->Color(red, green, blue));
}

void setAll(PixelStrip *strip, byte red, byte green, byte blue) {
  for (int i = 0; i < strip->numleds; i++) {
    setPixel(&(strip->strip), i, red, green, blue);
  }
  showStrip(&(strip->strip));
}

void meteorRain() {
  for (int k = 0; k < NUMSTRIPS; k++) {

    // end of meteor animation
    if (strips[k].currled > strips[k].numleds * 2) {
      strips[k].currled = 0;
      if (strips[k].endrandom) {
        strips[k].countdown = (rand() % (strips[k].randomenddelayend - strips[k].randomenddelaystart + 1)) + strips[k].randomenddelaystart;
      } else {
        strips[k].countdown = strips[k].enddelay;
      }

      setAll(&(strips[k]), strips[k].bgred, strips[k].bggreen, strips[k].bgblue);
      continue;
    }

    // delay countdown to start new meteor
    if (strips[k].countdown != 0) {
      //Serial.println(strips[k].countdown);
      strips[k].countdown--;
      continue;
    }

    // beginning of animation
    if (strips[k].beginrandom && strips[k].countdown == 0 && strips[k].currled == 0) {
      strips[k].beginrandom = false;
      strips[k].countdown = (rand() % (strips[k].randombegindelayend - strips[k].randombegindelaystart + 1)) + strips[k].randombegindelaystart;
      continue;
    }


    // fade brightness all LEDs one step
    for (int j = 0; j < strips[k].numleds; j++) {
      if ((!strips[k].meteorrandomdecay) || (random(10) > 3)) { // REDUCE LAST NUMBER TO PREVENT STUCK PIXELS ON FADE
        fadeToColor(&(strips[k].strip), j, strips[k].meteortraildecay, strips[k].bgred, strips[k].bggreen, strips[k].bgblue);
      }
    }


    // draw meteor
    for (int j = 0; j < strips[k].meteorsize; j++) {
      if ((strips[k].currled - j < strips[k].numleds) && (strips[k].currled - j >= 0)) {
        setPixel(&strips[k].strip, strips[k].currled - j, strips[k].red, strips[k].green, strips[k].blue);
      }
    }

    showStrip(&(strips[k].strip));
    strips[k].currled++;
    strips[k].countdown = strips[k].speeddelay;
  }
  //delay(200);
}