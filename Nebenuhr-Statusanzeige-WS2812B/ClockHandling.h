//=== ClockHandling.h for Nebenuhr-Statusanzeige-WS2812B=========
//=== declaration of out's ======================================

constexpr int16_t PIXEL_PIN_ORIGIN(10);   // this is used from Nebenuhr-Statusanzeige as output 
constexpr int16_t PIXEL_PIN(7);           // instead of Pin 10

//... ports are unused:
constexpr uint8_t TM1637CLK(4);
constexpr uint8_t TM1637DIO(5);
constexpr uint8_t RGB_R(9);
constexpr uint8_t RGB_G(11);
constexpr uint8_t RGB_B(3);
constexpr uint8_t LN_FAILED(12);       // LN-Ausfall, not used, cause this output from Nebenuhr-Statusanzeige is blinking
//... end ports are unused

//... Ports für FastClock-Status (Input):....................
constexpr uint8_t FC_NO_TELEGRAMM(13); // bisher kein Telegramm erhalten
constexpr uint8_t LN_WATCHER(A0);      // direct use of LN-Watcher-Hardware on Nebenuhr-Status
constexpr uint8_t FC_RATE_1_1(A1);     // Takt = 1:1
constexpr uint8_t FC_RATE_1_X(A2);     // Takt > 1:1
constexpr uint8_t FC_STOPPED(A3);      // Takt angehalten

#define PIXEL_COUNT           GetCV(ID_LED_COUNT)
#define PIXEL_BRIGHTNESS      GetCV(ID_LED_BRIGHT)
#define ENABLE_WHITE          (GetCV(ADD_FUNCTIONS_2) & 0x01)
#define ENABLE_LN_WATCHER     (GetCV(ADD_FUNCTIONS_2) & 0x80)
