//=== ClockHandling for Uhrenzentrale/Nebenuhr-Statusanzeige ===
#include "ClockHandling.h"

//................................................
#include <Adafruit_NeoPixel.h>

constexpr neoPixelType PIXEL_FORMAT(NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel pixels(NUMPIXELS, PIXEL_PIN, PIXEL_FORMAT);
boolean bPixels(false);
unsigned long ulWaitingForNextPixelsToSend(0);

//................................................
#include <TM1637Display.h>
TM1637Display display(TM1637CLK, TM1637DIO);

// For displays with just a colon: 00:00 (0b01000000)
constexpr uint8_t ui8_Colon(0b01000000);
constexpr uint8_t SEG_DASH[] ( {SEG_G, SEG_G, SEG_G, SEG_G } ); // set all 4 digits to '-'

//=== Functions =======================================
const boolean IsLnOk()
{ if(!ENABLE_LN_WATCHER)
    return true;
  return (!digitalRead(LN_WATCHER) ? true : false);
}

//................................................
void InitClockHandling()
{
/* pinMode for RGB-Outputs are set with analogWrite(...) */
  if (ENABLE_LN_WATCHER)
    pinMode(LN_WATCHER, INPUT_PULLUP);

  const uint16_t ui16_NumPixels(GetCV(ID_LED_COUNT));
  bPixels = (ui16_NumPixels > 0 ? true : false);
  if (bPixels)
  {
    pixels.updateLength(ui16_NumPixels);
    pixels.begin();
    pixels.clear(); // Set all pixel colors to 'off'
    pixels.setBrightness(GetCV(ID_LED_BRIGHT));  // only set once!  
  } // if (bPixelsEnabled)

  if (ENABLE_TM1637)
    display.setBrightness(0x0F);

  //=== diagnostic LEDs
  if (ENABLE_DIAGNOSTIC_LEDS)
  {
    pinMode(LN_FAILED, OUTPUT);
    pinMode(FC_NO_TELEGRAMM, OUTPUT);
    pinMode(FC_RATE_1_1, OUTPUT);
    pinMode(FC_RATE_1_X, OUTPUT);
    pinMode(FC_STOPPED, OUTPUT);
  } // if (ENABLE_DIAGNOSTIC_LEDS)
}

void HandleClockHandling()
{
  uint8_t ui8_Hour(0);
  uint8_t ui8_Minute(0);
  const boolean bIsFastClockStarted(GetFastClock(&ui8_Hour, &ui8_Minute));  // set if at least one telegramm received
  const boolean bIsRunning(isFastClockRunning()); // set if clockrate is not zero
  const uint8_t ui8Rate(GetClockRate()); // Rate: 0 = Freeze clock, 1 = normal, 10 = 10:1 etc. max is 0x7F
  const boolean bIsLnOk(IsLnOk());
  const boolean bRed(!bIsRunning);
  const boolean bGreen(bIsRunning && (ui8Rate > 1));
  const boolean bBlueOrWhite(bIsRunning && (ui8Rate == 1));
  const boolean bYellow(!bIsFastClockStarted);

  if (ENABLE_CLOCK_STOP_WITH_LN_WATCHER && !bIsLnOk)
    StopClock();
  
  if (IBNbyDisplayPanel())
  {
    const uint8_t iDisplayPanelMode(DisplayPanelMode());
    if ((iDisplayPanelMode >= MIN_COLOR) && (iDisplayPanelMode <= MAX_COLOR))
    {
      const boolean bRedIBN(iDisplayPanelMode == MIN_COLOR);
      const boolean bGreenIBN(iDisplayPanelMode == MIN_COLOR + 1);
      const boolean bBlueIBN(iDisplayPanelMode == MIN_COLOR + 2);
      const boolean bWhiteIBN(iDisplayPanelMode == MIN_COLOR + 3);
      const boolean bYellowIBN(iDisplayPanelMode == MIN_COLOR + 4);

      if (ENABLE_DIAGNOSTIC_LEDS)
      {
        digitalWrite(FC_STOPPED, bRedIBN);
        digitalWrite(FC_RATE_1_X, bGreenIBN);
        digitalWrite(LN_FAILED, bBlueIBN);
        digitalWrite(FC_RATE_1_1, bWhiteIBN);
        digitalWrite(FC_NO_TELEGRAMM, bYellowIBN);
      } // if (ENABLE_DIAGNOSTIC_LEDS)

      if (ENABLE_LED_STRIPES)
      {
        // always full brightness
        analogWrite(RGB_R, bRedIBN || bYellowIBN || bWhiteIBN? 255 : 0);
        analogWrite(RGB_G, bGreenIBN || bYellowIBN || bWhiteIBN? 255 : 0);
        analogWrite(RGB_B, bBlueIBN || bWhiteIBN? 255 : 0);
      } // if (ENABLE_LED_STRIPES)

      // WS2812B can show more than one status, e.g. FastClock not started
      if (bPixels)
      {
        // nur einmal alle PIXEL_DELAYms aktualisieren:
        if ((millis() - ulWaitingForNextPixelsToSend) >= PIXEL_DELAY)
        {
          if (bYellowIBN)
            pixels.fill(pixels.Color(255, 255, 0));
          else if (bRedIBN)
            pixels.fill(pixels.Color(255, 0, 0));
          else if (bGreenIBN)
            pixels.fill(pixels.Color(0, 255, 0));
          else if (bBlueIBN)
            pixels.fill(pixels.Color(0, 0, 255));
          else if (bWhiteIBN)
            pixels.fill(pixels.Color(255, 255, 255));

          // Send the updated pixels to the hardware
          // during send: Interrups are disabled
          // duration for 8 LEDs: about 270µs
          pixels.show(); 
          ulWaitingForNextPixelsToSend = millis();
        } // if ((millis() - ulWaitingForNextPixelsToSend) >= PIXEL_DELAY)
      } // if (bPixels)
      return; 
    } // if ((iDisplayPanelMode >= MIN_COLOR) && (iDisplayPanelMode <= MAX_COLOR))
  } // if (IBNbyDisplayPanel())

  //=== send to LED-Stripes
  if (ENABLE_LED_STRIPES)
  {
    const uint16_t ui16RBright(GetCV(ID_R_BRIGHT));
    if (!bIsLnOk)
    { // DCC / LocoNet failed (off)
      analogWrite(RGB_R, (Blinken2Hz() ? ui16RBright : 0));  // special handling for value 0 or 255 is done inside analogWrite(...) 
      analogWrite(RGB_G, 0);  // special handling for value 0 or 255 is done inside analogWrite(...) 
      analogWrite(RGB_B, 0);  // special handling for value 0 or 255 is done inside analogWrite(...)
    } // if (!IsLnOk())
    else
    { // DCC / LocoNet ok, with LED-Stripes each color represents only one information
      const boolean bUseWhite(bBlueOrWhite && ENABLE_WHITE);

      // red = fastclock not running;
      analogWrite(RGB_R, bRed || bYellow || bUseWhite ? ui16RBright : 0);  // special handling for value 0 or 255 is done inside analogWrite(...) 

      // green = fastclock running in fastclock-mode (quicker than realtime)
      const uint16_t ui16GBright(GetCV(ID_G_BRIGHT));
      analogWrite(RGB_G, bGreen || bYellow || bUseWhite ? ui16GBright: 0);  // special handling for value 0 or 255 is done inside analogWrite(...)

      // blue (white) = fastclock running in realtime (or slower)
      const uint16_t ui16BBright(GetCV(ID_B_BRIGHT));
      analogWrite(RGB_B, bBlueOrWhite || bUseWhite ? ui16BBright : 0);  // special handling for value 0 or 255 is done inside analogWrite(...)
    } // else if(!IsLnOk())
  } // if (ENABLE_LED_STRIPES)

  //=== send to WS2812B
  // WS2812B can show more than one status, e.g. FastClock not started
  if (bPixels)
  {
    // nur einmal alle PIXEL_DELAYms aktualisieren:
    if ((millis() - ulWaitingForNextPixelsToSend) >= PIXEL_DELAY)
    {
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

      // Send the updated pixels to the hardware
      // during send: Interrups are disabled
      // duration for 8 LEDs: about 270µs
      pixels.show(); 
      ulWaitingForNextPixelsToSend = millis();
    } // if ((millis() - ulWaitingForNextPixelsToSend) >= PIXEL_DELAY)
  } // if (bPixels)

  //=== diagnostic LEDs
  if (ENABLE_DIAGNOSTIC_LEDS)
  {
    digitalWrite(LN_FAILED, IsLnOk() ? 0 : Blinken2Hz());
    digitalWrite(FC_NO_TELEGRAMM, !bIsFastClockStarted);
    digitalWrite(FC_RATE_1_1, bBlueOrWhite);
    digitalWrite(FC_RATE_1_X, bGreen);
    digitalWrite(FC_STOPPED, bRed);
  } // if (ENABLE_DIAGNOSTIC_LEDS)

  //=== send time to display
  if (ENABLE_TM1637) // TM1637 connected
  {
    if (bIsFastClockStarted)
    {
      const uint16_t ui16_Time((ui8_Hour * 100) + ui8_Minute);
      display.showNumberDecEx(ui16_Time, !isFastClockRunning() || Blinken2Hz() ? ui8_Colon : 0);
    } // if (bIsFastClockStarted)
    else
      display.setSegments(SEG_DASH);
  } // if (ENABLE_TM1637)

}
