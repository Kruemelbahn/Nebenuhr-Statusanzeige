//=== ClockHandling for Nebenuhr-Statusanzeige ===
//=== declaration of out's =======================================
constexpr uint8_t RGB_R(9);
constexpr uint8_t RGB_G(11);
constexpr uint8_t RGB_B(3);

constexpr int16_t PIXEL_PIN(10);

constexpr uint8_t TM1637CLK(4);
constexpr uint8_t TM1637DIO(5);

constexpr uint8_t LN_WATCHER(A0);

#define ENABLE_WHITE      (GetCV(ADD_FUNCTIONS_2) & 0x01)
#define ENABLE_TM1637     (GetCV(ADD_FUNCTIONS_2) & 0x02)
#define ENABLE_LN_WATCHER (GetCV(ADD_FUNCTIONS_2) & 0x80)

//................................................
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

//................................................
#include <TM1637Display.h>
TM1637Display display(TM1637CLK, TM1637DIO);

// For displays with just a colon: 00:00 (0b01000000)
constexpr uint8_t ui8_Colon(0b01000000);
constexpr uint8_t SEG_DASH[]({SEG_G, SEG_G, SEG_G, SEG_G}); // set all 4 digits to '-'

//................................................
uint16_t ui16_NumPixels(0);

const boolean IsLnOk()
{ if(!ENABLE_LN_WATCHER)
    return true;
  return ((digitalRead(LN_WATCHER) == LOW) ? true : false);
};

//=== Functions =======================================
void InitClockHandling()
{
  ui16_NumPixels = GetCV(ID_LEDCOUNT);
  pixels.updateLength(ui16_NumPixels);
  pixels.begin();

/* pinMode for RGB-Outputs are set with analogWrite(...) */
  pinMode(LN_WATCHER, INPUT_PULLUP);
}

void HandleClockHandling()
{
  uint8_t ui8_Hour(0);
  uint8_t ui8_Minute(0);
  const boolean bIsFastClockStarted(GetFastClock(&ui8_Hour, &ui8_Minute));  // set if at least one telegramm received
  const boolean bIsRunning(isFastClockRunning()); // set if clockrate is not zero
  const uint8_t ui8Rate(GetClockRate());

  const boolean bRed(!bIsRunning);
  const boolean bGreen(bIsRunning && (ui8Rate > 1));
  const boolean bBlue(bIsRunning && (ui8Rate == 1));

  const uint16_t ui16RBright(GetCV(ID_R_BRIGHT));
  if(!IsLnOk())
  { // DCC / LocoNet failed (off)
    analogWrite(RGB_R, (Blinken2Hz() ? ui16RBright : 0));  // special handling for value 0 or 255 is done inside analogWrite(...) 
    analogWrite(RGB_G, 0);  // special handling for value 0 or 255 is done inside analogWrite(...) 
    analogWrite(RGB_B, 0);  // special handling for value 0 or 255 is done inside analogWrite(...) 
  }
  else
  { // DCC / LocoNet ok

    // fastclock not running;
    analogWrite(RGB_R, ui16RBright);  // special handling for value 0 or 255 is done inside analogWrite(...) 

    // fastclock running in fastclock-mode (quicker than realtime)
    const uint16_t ui16GBright(GetCV(ID_G_BRIGHT));
    analogWrite(RGB_G, ui16GBright);  // special handling for value 0 or 255 is done inside analogWrite(...)

    // fastclock running in realtime (or slower)
    const uint16_t ui16BBright(GetCV(ID_B_BRIGHT));
    analogWrite(RGB_B, ui16BBright);  // special handling for value 0 or 255 is done inside analogWrite(...)  
  }

  // send to WS2812B
  if(ui16_NumPixels)
  {
    const uint16_t ui16LEDBright(GetCV(ID_LED_BRIGHT));
    pixels.setBrightness(ui16LEDBright & 0xFF);  
    for(int i = 0; i < ui16_NumPixels; i++)
    {
      if(!IsLnOk())
        // DCC / LocoNet failed (off)
        pixels.setPixelColor(i, pixels.Color((Blinken2Hz() ? 255 : 0), 0, 0));
      else
      { // DCC / LocoNet ok
        if(bIsFastClockStarted || !(bRed || bGreen || bBlue))
        {
          if(bRed)
            pixels.setPixelColor(i, pixels.Color(255, 0, 0));
          else if(bGreen)
            pixels.setPixelColor(i, pixels.Color(0, 255, 0));
          else if(bBlue)
          {
            if(ENABLE_WHITE) // use white instead blue
              pixels.setPixelColor(i, pixels.Color(255, 255, 255));
            else
              pixels.setPixelColor(i, pixels.Color(0, 0, 255));
          }
        }
        else
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
      pixels.show(); // This sends the updated pixel color to the hardware.
    } // for(int i = 0; i < ui16_NumPixels; i++)
  } // if(ui16_NumPixels)

  if(ENABLE_TM1637) // TM1637 connected
  {
    const uint16_t ui16_Time((ui8_Hour * 100) + ui8_Minute);

    display.setBrightness(0x0F);
    if(bIsFastClockStarted)
      display.showNumberDecEx(ui16_Time, !isFastClockRunning() || Blinken2Hz() ? ui8_Colon : 0);
    else
      display.setSegments(SEG_DASH);
  }
}
