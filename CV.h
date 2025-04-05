//=== CV-Definitions for Nebenuhr-Statusanzeige ===

// give CV a unique name
enum { ID_DEVICE = 0, ID_LEDCOUNT, ID_LED_BRIGHT, ID_R_BRIGHT, ID_G_BRIGHT, ID_B_BRIGHT, VERSION_NUMBER, SOFTWARE_ID, ADD_FUNCTIONS_1, ADD_FUNCTIONS_2
#if defined ETHERNET_BOARD
     , IP_BLOCK_3, IP_BLOCK_4
#endif
};

//=== declaration of var's =======================================
#define PRODUCT_ID SOFTWARE_ID
static const uint8_t DEVICE_ID = 1;							// CV1: Device-ID
static const uint8_t SW_VERSION = 3;						// CV7: Software-Version
static const uint8_t SLAVE_CLOCK_STATUS = 18;		// CV8: Software-ID

#if defined ETHERNET_BOARD
  static const uint8_t MAX_CV = 12;
#else
  static const uint8_t MAX_CV = 10;
#endif

uint16_t ui16a_CV[MAX_CV] = { UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX
#if defined ETHERNET_BOARD
                          , UINT16_MAX, UINT16_MAX
#endif
                          };  // ui16a_CV is a copy from eeprom

struct _CV_DEF // uses 9 Byte of RAM for each CV
{
  uint8_t ID;
  uint16_t DEFLT;
  uint16_t MIN;
  uint16_t MAX;
  uint8_t TYPE;
  boolean RO;
};

enum CV_TYPE { UI8 = 0, UI16 = 1, BINARY = 2 };

const struct _CV_DEF cvDefinition[MAX_CV] =
{ // ID               default value       minValue			        maxValue            type              r/o
   { ID_DEVICE,       DEVICE_ID,          1,						        126,                CV_TYPE::UI8,     false}  // normally r/o
  ,{ ID_LEDCOUNT,     NUMPIXELS,          1,						        255,                CV_TYPE::UI8,     false}  // count of WS2812B
  ,{ ID_LED_BRIGHT,   255,                0,						        255,                CV_TYPE::UI8,     false}  // Brightness for WS2812B
  ,{ ID_R_BRIGHT,     255,                0,						        255,                CV_TYPE::UI8,     false}  // Brightness for RED
  ,{ ID_G_BRIGHT,     255,                0,						        255,                CV_TYPE::UI8,     false}  // Brightness for GREEN
  ,{ ID_B_BRIGHT,     255,                0,						        255,                CV_TYPE::UI8,     false}  // Brightness for BLUE
  ,{ VERSION_NUMBER,  SW_VERSION,         0,						        SW_VERSION,         CV_TYPE::UI8,     false}  // normally r/o
  ,{ SOFTWARE_ID,     SLAVE_CLOCK_STATUS, SLAVE_CLOCK_STATUS,   SLAVE_CLOCK_STATUS, CV_TYPE::UI8,     true}   // always r/o
  ,{ ADD_FUNCTIONS_1, 0b00011000,         0,						        UINT8_MAX,          CV_TYPE::BINARY,  false}  // additional functions 1
  ,{ ADD_FUNCTIONS_2, 0b00000000,         0,						        UINT8_MAX,          CV_TYPE::BINARY,  false}  // additional functions 2
#if defined ETHERNET_BOARD
  ,{ IP_BLOCK_3,      2,                  0,						        UINT8_MAX,          CV_TYPE::UI8,     false}  // IP-Address part 3
  ,{ IP_BLOCK_4,      106,                0,						        UINT8_MAX,          CV_TYPE::UI8,     false}  // IP-Address part 4
#endif
};

//=== naming ==================================================
const __FlashStringHelper* GetSwTitle() { return F("Nebenuhr-Status"); }
//========================================================
const __FlashStringHelper *GetCVName(uint8_t ui8_Index)
{ 
  // each string should have max. 10 chars
  const __FlashStringHelper *cvName[MAX_CV] = { F("DeviceID"),
                                                F("LED-CNT"),
                                                F("WS-Bright"),
                                                F("R-Bright"),
                                                F("G-Bright"),
                                                F("B-Bright"),
                                                F("Version"),
                                                F("SW-ID"),
                                                F("Cfg FC-Slv"),
                                                F("Config l")
#if defined ETHERNET_BOARD
                                              , F("IP-Part 3")
                                              , F("IP-Part 4")
#endif
                                                };
                                    
  if(ui8_Index < MAX_CV)
    return cvName[ui8_Index];
  return F("???");
}

//=== functions ==================================================
boolean AlreadyCVInitialized() { return (ui16a_CV[SOFTWARE_ID] == SLAVE_CLOCK_STATUS); }
