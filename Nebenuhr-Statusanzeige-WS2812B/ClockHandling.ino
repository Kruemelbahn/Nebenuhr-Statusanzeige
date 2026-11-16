//=== ClockHandling.cpp for Nebenuhr-Statusanzeige-WS2812B=========
#include "ClockHandling.h"

//................................................
#include <Adafruit_NeoPixel.h>

constexpr neoPixelType PIXEL_FORMAT(NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel pixels(NUMPIXELS, PIXEL_PIN, PIXEL_FORMAT);

//=== Functions =======================================
const boolean IsLnOk()
{ 
  if(!ENABLE_LN_WATCHER)
    return true;
  return (!digitalRead(LN_WATCHER) ? true : false);
}

void InitClockHandling()
{
  //... unused ports, set to input to avoid collisions:
  pinMode(TM1637CLK, INPUT_PULLUP);
  pinMode(TM1637DIO, INPUT_PULLUP);
  pinMode(RGB_R, INPUT_PULLUP);
  pinMode(RGB_G, INPUT_PULLUP);
  pinMode(RGB_B, INPUT_PULLUP);
  pinMode(PIXEL_PIN_ORIGIN, INPUT_PULLUP);
  pinMode(LN_FAILED, INPUT_PULLUP);
  //... end unused ports

  pinMode(LN_WATCHER, INPUT_PULLUP);
  pinMode(FC_NO_TELEGRAMM, INPUT_PULLUP);
  pinMode(FC_RATE_1_1, INPUT_PULLUP);
  pinMode(FC_RATE_1_X, INPUT_PULLUP);
  pinMode(FC_STOPPED, INPUT_PULLUP);

  pixels.updateLength(PIXEL_COUNT); // only set once!
  pixels.begin();
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.setBrightness(PIXEL_BRIGHTNESS); // only set once!  
}

void HandleClockHandling()
{
  const boolean bIsLnOk(IsLnOk());
  const boolean bRed(digitalRead(FC_STOPPED) ? true : false);
  const boolean bGreen(digitalRead(FC_RATE_1_X) ? true : false);
  const boolean bBlueOrWhite(digitalRead(FC_RATE_1_1) ? true : false);
  const boolean bYellow(digitalRead(FC_NO_TELEGRAMM) ? true : false);

  if (!bIsLnOk)
    // DCC / LocoNet failed (off)
    pixels.fill(pixels.Color((Blinken2Hz() ? 255 : 0), 0, 0)); // red / aus
  else if (bYellow)
    pixels.fill(pixels.Color(255, 255, 0));
  else if (bRed)
    pixels.fill(pixels.Color(255, 0, 0));
  else if (bGreen)
    pixels.fill(pixels.Color(0, 255, 0));
  else if (bBlueOrWhite)
  {
    if(ENABLE_WHITE) // use white instead blue
      pixels.fill(pixels.Color(255, 255, 255));
    else
      pixels.fill(pixels.Color(0, 0, 255));
  } // else if (bBlueOrWhite)
  else
    // kein Status gesetzt, d.h. alle Eingänge haben LOW (GND)
    // pixels.clear(); // don't set all pixel colors to 'off', we want to have a color in this case
    pixels.fill(pixels.Color(128, 0, 128)); // (darker) violett

  // Send the updated pixels to the hardware
  // during send: Interrups are disabled
  // duration for 8 LEDs: about 270µs
  pixels.show(); 
}
