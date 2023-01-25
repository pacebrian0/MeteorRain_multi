/*  Copyright (c) 2022 Brian J Pace
    Email: pacebrian0@gmail.com
*/

#include <Adafruit_NeoPixel.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
typedef struct PixelStrip
{
  Adafruit_NeoPixel strip;
  byte red;
  byte green;
  byte blue;
  byte meteorsize;
  byte meteortraildecay;
  bool meteorrandomdecay;
  byte speeddelay;
  int16_t currled;
  int16_t numleds;
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
  byte fadeinlength;
  byte fadeinstrength;
  bool reverse;
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
enddelay: how long it takes to start a new meteor after one finishes->
randomenddelaystart: minimum time taken to start a new meteor
randomenddelayend: maximum time taken to start a new meteor
beginrandom: determines if the first meteor after being switched on will be delayed with a random number between randombegindelaystart and randombegindelayend
randombegindelaystart: minimum time taken to start first meteor
randombegindelayend: maximum time taken to start first meteor
bgred, bggreen, bgblue: background colours for LED strip
fadeinlength: bigger number = number of LEDS for meteor fade-in
fadeinstrength: 0-255 how bright is the fade-in. Bigger number = brighter
reverse: set to true to reverse the animation
*************************************************************/

PixelStrip strips[] = {
    // strip            leds  pin                           red   green blue  meteorsize  meteortraildecay  meteorrandomdecay speeddelay  currled(0)  numleds countdown(0)  endrandom   enddelay  randomenddelaystart   randomenddelayend   beginrandom   randombegindelaystart   randombegindelayend   bgred   bggreen   bgblue fadeinlength fadeinstrength reverse
    {Adafruit_NeoPixel(40, 6, NEO_GRB + NEO_KHZ800), 150, 75, 30, 1, 16, false, 50, 0, 40, 0, false, 500, 0, 500, false, 0, 5000, 16, 8, 4, 2, 127, false},
    //{ Adafruit_NeoPixel(40, 12, NEO_GRB + NEO_KHZ800), 210, 120, 50, 1, 64, true, 20, 0, 40, 0, false, 20, 0, 500, false, 0, 5000, 0, 0, 0, 2, 50, false },
    //{ Adafruit_NeoPixel(40, 7, NEO_GRB + NEO_KHZ800), 255, 255, 10, 1, 64, true, 60, 0, 40, 0, true, 20, 0, 500, true, 0, 5000, 10, 10, 10, 2, 50, false },
    //{ Adafruit_NeoPixel(40, 8, NEO_GRB + NEO_KHZ800), 255, 10, 10, 1, 64, true, 60, 0, 40, 0, true, 20, 0, 500, true, 0, 5000, 10, 10, 10,  2, 50, false },
    //{ Adafruit_NeoPixel(40, 9, NEO_GRB + NEO_KHZ800), 10, 10, 255, 1, 64, true, 60, 0, 40, 0, true, 20, 0, 500, true, 0, 5000, 10, 10, 10,  2, 50, false },
    //{ Adafruit_NeoPixel(40, 10, NEO_GRB + NEO_KHZ800), 10, 10, 255, 1, 64, true, 60, 0, 40, 0, true, 20, 0, 500, true, 0, 5000, 10, 10, 10, 2, 50, false },
    {Adafruit_NeoPixel(40, 11, NEO_GRB + NEO_KHZ800), 255, 255, 255, 1, 64, true, 60, 0, 40, 0, false, 20, 0, 500, false, 0, 5000, 2, 2, 2, 2, 32, true},
    //{ Adafruit_NeoPixel(40, 13, NEO_GRB + NEO_KHZ800), 10, 255, 10, 1, 64, true, 50, 0, 40, 0, true, 500, 0, 500, true, 0, 5000, 10, 10, 10,  2, 50, false },

};

// Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

#define NUMSTRIPS (sizeof(strips) / sizeof(strips[0]))

// speed for screen wipe at the beginning
#define WIPESPEED 50

// enable controller trigger
#define REMOTE true

// controller pin 
#define REMOTEPIN 1

bool STARTANIMATION = false;
bool ANIMATIONINPROGRESS = false;
unsigned long previousTime = 0;  // to measure loop time per millisecond precisely
unsigned long currentTime = 0;  // to keep track of current ms

void setup()
{
  // Serial.begin(115200);
  byte max_numleds = 0;

  if (REMOTE)
    pinMode(REMOTEPIN, INPUT);

  for (int i = 0; i < NUMSTRIPS; i++)
  {
    if (strips[i].numleds > max_numleds)
      max_numleds = strips[i].numleds;
  }

  for (int i = 0; i < NUMSTRIPS; i++)
  {
    strips[i].strip.begin();
    strips[i].strip.show(); // Initialize all pixels to 'off'
    setAll(&(strips[i]), 0, 0, 0);
  }

  // perform screen wipe
  for (int i = 0; i < max_numleds; i++)
  {
    screenWipe();
    delay(WIPESPEED);
  }

  // reset led counter
  for (int i = 0; i < NUMSTRIPS; i++)
  {
    strips[i].currled = 0;
  }
  delay(1000);
}

void screenWipe()
{
  for (int k = 0; k < NUMSTRIPS; k++)
  {
    PixelStrip* s = &strips[k];
    // end of meteor animation
    if (s->currled > s->numleds)
    {
      continue;
    }

    // draw screen
    if ((s->currled <= s->numleds) && (s->currled >= 0))
    {
      if (s->reverse)
        setPixel(&(s->strip), s->numleds - s->currled - 1, s->bgred, s->bggreen, s->bgblue);
      else
        setPixel(&(s->strip), s->currled, s->bgred, s->bggreen, s->bgblue);
    }

    showStrip(&(s->strip));
    s->currled++;
  }
}

void loop()
{

  if (!STARTANIMATION)
    STARTANIMATION = digitalRead(REMOTEPIN) == HIGH ? true : false;

  // Serial.println(sizeof(strips));
  // Serial.println(sizeof(strips[0]));
  

  unsigned long currentTime = millis();
  if (currentTime > previousTime)
  {
    meteorRain();
    previousTime = currentTime;
  }
}

void printColor(Adafruit_NeoPixel *strip, int ledNo)
{
  int32_t oldColor;
  byte r, g, b;
  int value;

  oldColor = strip->getPixelColor(ledNo);
  r = (oldColor & 0x00ff0000UL) >> 16;
  g = (oldColor & 0x0000ff00UL) >> 8;
  b = (oldColor & 0x000000ffUL);
  Serial.print(r);
  Serial.print(" ");
  Serial.print(g);
  Serial.print(" ");
  Serial.print(b);
  Serial.print("\t");
}

void fadeToColor(Adafruit_NeoPixel *strip, int ledNo, byte fadeValue, byte bgred, byte bggreen, byte bgblue)
{

  int32_t oldColor;
  byte r, g, b;
  int value;

  oldColor = strip->getPixelColor(ledNo);
  r = (oldColor & 0x00ff0000UL) >> 16;
  g = (oldColor & 0x0000ff00UL) >> 8;
  b = (oldColor & 0x000000ffUL);

  r = (r <= bgred) ? bgred : (int)r - (r * fadeValue / 256) - 1;
  g = (g <= bggreen) ? bggreen : (int)g - (g * fadeValue / 256) - 1;
  b = (b <= bgblue) ? bgblue : (int)b - (b * fadeValue / 256) - 1;
  // if(ledNo == 20){
  // Serial.print(r);
  // Serial.print(" ");
  // Serial.print(g);
  // Serial.print(" ");
  // Serial.print(b);
  // Serial.println(" ");
  // }
  strip->setPixelColor(ledNo, r, g, b);
}

void showStrip(Adafruit_NeoPixel *strip)
{
  strip->show();
}



void setPixel(Adafruit_NeoPixel *strip, int Pixel, byte red, byte green, byte blue)
{
  strip->setPixelColor(Pixel, strip->Color(red, green, blue));
}

void setAll(PixelStrip *strip, byte red, byte green, byte blue)
{
  for (int i = 0; i < strip->numleds; i++)
  {
    setPixel(&(strip->strip), i, red, green, blue);
  }
  showStrip(&(strip->strip));
}

void applyGainStep(Adafruit_NeoPixel *strip, int ledNo, byte bgred, byte bggreen, byte bgblue, byte r, byte g, byte b, byte currStep, byte numSteps, byte str)
{
  byte red = bgred + ((((r - bgred) / (numSteps + 1)) * currStep * str) / 256);
  byte green = bggreen + ((((g - bggreen) / (numSteps + 1)) * currStep * str) / 256);
  byte blue = bgblue + ((((b - bgblue) / (numSteps + 1)) * currStep * str) / 256);

  // if(ledNo == 20)
  // {
  // Serial.print("\t");
  // Serial.print(currStep);
  // Serial.print(" ");
  // Serial.print(red);
  // Serial.print(" ");
  // Serial.print(green);
  // Serial.print(" ");
  // Serial.print(blue);
  // }

  strip->setPixelColor(ledNo, red, green, blue);
}

void meteorRain()
{

  for (int k = 0; k < NUMSTRIPS; k++)
  {
    PixelStrip* s = &strips[k];
    // end of meteor animation
    if (s->currled > (s->numleds * 2) + s->fadeinlength)
    {
      s->currled = 0;
      if (REMOTE)
      {
        STARTANIMATION = false;
        ANIMATIONINPROGRESS = false;
      }
      else
      {
        if (s->endrandom)
        {
          s->countdown = (rand() % (s->randomenddelayend - s->randomenddelaystart + 1)) + s->randomenddelaystart;
        }
        else
        {
          s->countdown = s->enddelay;
        }
      }

      setAll(s, s->bgred, s->bggreen, s->bgblue);
      continue;
    }

    // delay countdown to start new meteor
    if (s->countdown != 0)
    {
      // Serial.println(s->countdown);
      s->countdown--;
      continue;
    }

    // controls for remote
    if (REMOTE)
    {
      if (!ANIMATIONINPROGRESS && !STARTANIMATION) // animation finished and waiting for pin trigger
        continue;

      if (ANIMATIONINPROGRESS && STARTANIMATION) // pin trigger happened during animation, ignore
        STARTANIMATION = false;

      if (!ANIMATIONINPROGRESS && STARTANIMATION) // time to start animation!
        ANIMATIONINPROGRESS = true;

      // if(ANIMATIONINPROGRESS && !STARTANIMATION) // normal behaviour during animation
    }

    // beginning of random animation, extend countdown with random ticks
    if (s->beginrandom && s->countdown == 0 && s->currled == 0)
    {
      s->beginrandom = false;
      s->countdown = (rand() % (s->randombegindelayend - s->randombegindelaystart + 1)) + s->randombegindelaystart;
      continue;
    }

    // Serial.print(s->currled);
    // Serial.print(" ");
    // Serial.print(s->red);
    // Serial.print(" ");
    // Serial.print(s->green);
    // Serial.print(" ");
    // Serial.print(s->blue);
    // Serial.print("\t");
    // Serial.print(s->bgred);
    // Serial.print(" ");
    // Serial.print(s->bggreen);
    // Serial.print(" ");
    // Serial.print(s->bgblue);
    // Serial.print(" ");
    // Serial.print(s->reverse?"r":"");
    // Serial.print(" ");
    // Serial.print(s->reverse?s->numleds - s->currled - 1:0);

    // Serial.print("\t");
    // printColor(&(s->strip),35);
    // Serial.println(" ");

    // fade brightness all LEDs one step
    for (int j = 0; j < s->currled - s->meteorsize - s->fadeinlength; j++)
    {
      // Serial.print(s->currled);
      if ((!s->meteorrandomdecay) || (random(10) > 3))
      { // REDUCE LAST NUMBER TO PREVENT STUCK PIXELS ON FADE

        if (s->reverse)
          fadeToColor(&(s->strip), s->numleds - j - 1, s->meteortraildecay, s->bgred, s->bggreen, s->bgblue);
        else
          fadeToColor(&(s->strip), j, s->meteortraildecay, s->bgred, s->bggreen, s->bgblue);
      }
    }

    // Serial.print(s->red);
    // Serial.print(" ");
    // Serial.print(s->green);
    // Serial.print(" ");
    // Serial.print(s->blue);
    // Serial.println(" ");
    // draw meteor
    for (int j = 0; j < s->meteorsize + s->fadeinlength; j++)
    {
      if ((s->currled - j < s->numleds) && (s->currled - j >= 0))
      {
        // Serial.print(s->currled);

        if (j < s->fadeinlength)
        {
          // Serial.print(" gaining RGB ");
          // Serial.print(j);
          // Serial.print(": ");
          if (s->reverse)
            applyGainStep(&(s->strip), s->numleds - s->currled - 1 + j, s->bgred, s->bggreen, s->bgblue, s->red, s->green, s->blue, j + 1, s->fadeinlength, s->fadeinstrength);
          else
            applyGainStep(&(s->strip), s->currled - j, s->bgred, s->bggreen, s->bgblue, s->red, s->green, s->blue, j + 1, s->fadeinlength, s->fadeinstrength);
          // gainToColor(&(s->strip), s->currled - j, 64, s->red, s->green, s->blue);
        }
        else
        {
          if (s->currled - j == 20)
          {
            // Serial.print("\t meteor \t");
            // Serial.print(s->red);
          }
          if (s->reverse)
            setPixel(&s->strip, s->numleds - s->currled + j - 1, s->red, s->green, s->blue);
          else
            setPixel(&s->strip, s->currled - j, s->red, s->green, s->blue);
          // printColor(&s->strip, s->currled - j);
        }
      }
    }

    showStrip(&(s->strip));
    s->currled++;
    s->countdown = s->speeddelay;
    // Serial.println(" ");
  }
  // delay(200);
}