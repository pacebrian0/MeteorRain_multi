/*  Copyright (c) 2022 Brian J Pace
    Email: pacebrian0@gmail.com
*/
#include <TimerOne.h>
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
  byte fadeoutlength;
  byte fadeoutstrength;
  bool reverse;
  int ledGroup;
  bool remoteStart;  
  bool remoteAnimation;
} PixelStrip;

/*************************************************************
strip: defines a new LED strip, needs the following parameters: number of LEDs and PIN, leave the NEO_GRB + NEO_KHZ800 as is
red, green, blue: RGB values for strip colour
meteorsize: number of LEDS that the bright meteor will be displayed as
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
fadeoutlength: bigger number = number of LEDS for meteor fade-out
fadeoutstrength: 0-255 how bright is the fade-out. Bigger number = brighter
reverse: set to true to reverse the animation
ledGroup: used to group up different led strips to begin together e.g. strip 1 & 2, strip 3 & 4... Begin from 0
remoteStart: used to determine when to start animation when using remote trigger. Leave as-is
remoteAnimation: used to determine if animation is in progress when using remote trigger. Leave as-is
*************************************************************/

PixelStrip strips[] = {
    // strip            leds  pin                    Red  green blue  meteorsize  speeddelay  currled(0)  numleds countdown(0)  endrandom   enddelay  randomenddelaystart   randomenddelayend   beginrandom   randombegindelaystart   randombegindelayend   bgred   bggreen   bgblue  fadeinlength  fadeinstrength  fadeoutlength fadeoutstrength reverse   ledGroup  remoteStart remoteAnimation
    {Adafruit_NeoPixel(40, 7, NEO_GRB + NEO_KHZ800), 150, 75,   30,   1,          50,         0,          40,     0,            false,      0,        0,                    500,                false,        0,                      5000,                 16,     8,        4,      2,            127,            2,            127,            false,    0,        false,      false},
    {Adafruit_NeoPixel(40, 6, NEO_GRB + NEO_KHZ800), 255, 255,  255,  1,          60,         0,          40,     0,            false,      0,        0,                    500,                false,        0,                      5000,                 2,      2,        2,      5,            32,             5,            32,             false,    1,        false,      false},


};

// Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

#define NUMSTRIPS (sizeof(strips) / sizeof(strips[0]))

// speed for screen wipe at the beginning
#define WIPESPEED 50

// enable controller trigger
#define REMOTE true

// controller pin 
#define REMOTEPIN 3

byte maxGroups = 0;
unsigned long previousTime = 0;  // to measure loop time per millisecond precisely
unsigned long currentTime = 0;  // to keep track of current ms
int currLedGroup = 0;
//bool waitForFinish = false; //if true, strips will wait for everyone to be ready before initiating next loop, ignores resetTimer
bool triggered = false; // checks if the current HIGH input in pin has been acted upon, if false`, it will accept next input
unsigned long triggeredTime = 0; // keeps track of the time passed since last trigger
unsigned long triggeredWait = 4; // ms to wait until waiting for next press (to prevent double-triggers)

//PERFORMACE
// unsigned long runsPerSec=0;
// unsigned long prevrun=0;

void setup()
{
  //Serial.begin(115200);
  int max_numleds = 0;
  Timer1.initialize(1000); // set the timer to trigger every 1 millisecond (1000 microseconds)
  Timer1.attachInterrupt(timerIsr); // attach the interrupt service routine (ISR)
  if (REMOTE)
  {
    pinMode(REMOTEPIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
  }
  for (int i = 0; i < NUMSTRIPS; i++)
  {
    if (strips[i].numleds > max_numleds)
      max_numleds = strips[i].numleds;

    if (strips[i].ledGroup > maxGroups)
      maxGroups = strips[i].ledGroup;
      
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

void timerIsr() {
  currentTime++;
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



void checkPin()
{
  //if(digitalRead(REMOTEPIN) == HIGH)
  //Serial.println(digitalRead(REMOTEPIN));  
  if (digitalRead(REMOTEPIN) == HIGH && !triggered && ((unsigned long)(currentTime - triggeredTime) > triggeredWait))
  {

    //Serial.print("Triggered from pin ");
    //Serial.println(REMOTEPIN);

    // The button is pressed, do something!
    triggered = true;
    triggeredTime=currentTime;
    digitalWrite(LED_BUILTIN, HIGH);  // Onboard LED feedback

  }

  if (triggered) //time to reset trigger
  {
    if(currLedGroup > maxGroups) // time to reset loop
    {
      currLedGroup = 0;
    }
      
    // Serial.print("Triggered from pin ");
    // Serial.print(REMOTEPIN);
    // Serial.print("\t");
    // Serial.println(currLedGroup);
    for (int i = 0; i < NUMSTRIPS; i++)
    {
      
      if (strips[i].ledGroup == currLedGroup && strips[i].remoteAnimation == false)
      {
        
        strips[i].remoteStart = true;
      }
        
    }
    
    currLedGroup++;    
    triggered = false;
    digitalWrite(LED_BUILTIN, LOW);  // Onboard LED feedback
         
  }

}

void loop()
{
  //currentTime = millis();
  if(REMOTE)
    checkPin();  
   
  if (currentTime - previousTime > 0) // any number other than 0 will stack multiplicatively with the speed modifier 
  {
    
    meteorRain();
    
    previousTime = currentTime;
  }

  // PERFORMANCE  
  // if((unsigned long)(currentTime - prevrun) > resetTimer)
  // {
  //   Serial.println(currentTime - prevrun); 
  //   Serial.println(runsPerSec); 
  //   prevrun=currentTime;
  //   runsPerSec=0;
  // }
  // runsPerSec++;  
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

  // if(ledNo == 0)
  // {
  // Serial.print("\t");
  // Serial.print(ledNo);
  // Serial.print(" ");
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
    // Serial.print(s->currled);
    // Serial.print(" ");
    // Serial.print(s->remoteAnimation);
    // Serial.print(" ");
    // Serial.print(s->remoteStart);
    // Serial.print("  ");
    // Serial.println(s->countdown);
    
    //Serial.print(currLedGroup);
    //Serial.print(" ");
    //Serial.print(s->remoteAnimation);
    // Serial.print("\t");
    // printColor(&(s->strip),35);
    // Serial.println(" ");
    // end of meteor animation
    if (s->currled > s->numleds + s->fadeinlength + s->fadeoutlength + s->meteorsize)
    {
      s->currled = 0;
      if (REMOTE)
      {
        s->remoteStart = false;
        s->remoteAnimation = false;
        //Serial.println("end start:0, anim:0");
        
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
      if (!s->remoteAnimation && !s->remoteStart) // animation finished and waiting for pin trigger
        continue;

      if (s->remoteAnimation  && s->remoteStart) // pin trigger happened during animation, ignore
      {
        s->remoteStart = false;
        //Serial.println("start:0");        
      }
        
      if (!s->remoteAnimation  && s->remoteStart) // time to start animation!
      {
        //Serial.println(" Starting now!");
        s->remoteAnimation  = true;
        //Serial.println("anim:1");     
      }
      // if(s->remoteAnimation  && !s->remoteStart) // normal behaviour during animation
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


    // Serial.print(s->red);
    // Serial.print(" ");
    // Serial.print(s->green);
    // Serial.print(" ");
    // Serial.print(s->blue);
    // Serial.println(" ");

    // draw fade-in + meteor
    for (int j = 0; j < s->meteorsize + s->fadeinlength; j++)
    {
      if ((s->currled - j < s->numleds) && (s->currled - j >= 0))
      {
        // Serial.print(s->currled);

        if (j < s->fadeinlength)
        {
          // Serial.print(" gaining RGB ");
          // if(s->reverse)    
          //   Serial.print(s->numleds - s->currled - 1 + j);
          // else
          //   Serial.print(s->currled - j);
          // Serial.print(": ");
          if (s->reverse)
            applyGainStep(&(s->strip), s->numleds - s->currled - 1 + j, s->bgred, s->bggreen, s->bgblue, s->red, s->green, s->blue, j + 1, s->fadeinlength , s->fadeinstrength);
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
          {
            // Serial.print(" meteor at ");   
            // Serial.print(s->numleds - s->currled + j - 1);     
            setPixel(&s->strip, s->numleds - s->currled + j - 1, s->red, s->green, s->blue);
          }
          else
          {
            // Serial.print(" meteor at ");   
            // Serial.print(s->currled - j);            
            setPixel(&s->strip, s->currled - j, s->red, s->green, s->blue);
          }
          // printColor(&s->strip, s->currled - j);
        }
      }
    }


  //draw fade-out
    for (int j = 0; j <= s->fadeoutlength; j++)
    {

      if ((s->currled - s->meteorsize - s->fadeinlength - j < s->numleds) && (s->currled - s->meteorsize - s->fadeinlength - j >= 0))
      {
        // Serial.print(s->currled);  

        if (j < s->fadeoutlength)
        {
          // Serial.print(" losing RGB ");
          // if(s->reverse)    
          //   Serial.print(s->numleds - s->currled + s->meteorsize + 1 + j );  
          // else
          //   Serial.print(s->currled - s->meteorsize - s->fadeinlength - j );
          // Serial.print(": ");
          
          if (s->reverse)
            //applyFadeStep(&(s->strip),s->numleds - s->currled + s->meteorsize + 1  + j , s->bgred, s->bggreen, s->bgblue, s->red, s->green, s->blue, j + 1 , s->fadeoutlength, s->fadeoutstrength);
            applyGainStep(&(s->strip), s->numleds - s->currled + s->meteorsize + 1 + j, s->bgred, s->bggreen, s->bgblue, s->red, s->green, s->blue, s->fadeoutlength -j , s->fadeoutlength, s->fadeoutstrength);

          else
            applyGainStep(&(s->strip), s->currled - s->meteorsize - s->fadeinlength - j , s->bgred, s->bggreen, s->bgblue, s->red, s->green, s->blue, s->fadeoutlength -j, s->fadeoutlength, s->fadeoutstrength);
            //applyFadeStep(&(s->strip), s->currled - s->meteorsize - s->fadeinlength - j , s->bgred, s->bggreen, s->bgblue, s->red, s->green, s->blue, j + 1 , s->fadeoutlength, s->fadeoutstrength);

          // gainToColor(&(s->strip), s->currled - j, 64, s->red, s->green, s->blue);
        }
        else
        {
          if (s->reverse)
          {
            // Serial.print(" bg at ");   
            // Serial.print(s->numleds - s->currled + s->meteorsize + 1 + j);     
            setPixel(&s->strip, s->numleds - s->currled + s->meteorsize + 1 + j, s->bgred, s->bggreen, s->bgblue);
          }
            
          else
          {
            // Serial.print(" bg at ");   
            // Serial.print(s->currled - s->meteorsize - s->fadeinlength - j);  
            setPixel(&s->strip, s->currled - s->meteorsize - s->fadeinlength - j, s->bgred, s->bggreen, s->bgblue);
            
          }
          // printColor(&s->strip, s->currled - j);
          
        }

      }
    }

    showStrip(&(s->strip));
    s->currled++;
    s->countdown = s->speeddelay;
    // Serial.println("");
  }
  // delay(200);
}