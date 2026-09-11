//=== ClockHandling.h for Nebenuhr-Statusanzeige =================
//=== declaration of out's =======================================

constexpr int16_t PIXEL_PIN(10);
constexpr unsigned long PIXEL_DELAY(250);

constexpr uint8_t TM1637CLK(4);
constexpr uint8_t TM1637DIO(5);

// PWM-ports are used...
constexpr uint8_t RGB_R(9);
constexpr uint8_t RGB_G(11);
constexpr uint8_t RGB_B(3);

constexpr uint8_t LN_WATCHER(A0);

#define ENABLE_WHITE              (GetCV(ADD_FUNCTIONS_2) & 0x01)
#define ENABLE_TM1637             (GetCV(ADD_FUNCTIONS_2) & 0x02)
#define ENABLE_DIAGNOSTIC_LEDS    (GetCV(ADD_FUNCTIONS_2) & 0x04)
#define ENABLE_LED_STRIPES        (GetCV(ADD_FUNCTIONS_2) & 0x08)
#define ENABLE_LN_WATCHER         (GetCV(ADD_FUNCTIONS_2) & 0x80)
#define ENABLE_CLOCK_STOP_WITH_LN_WATCHER (0)

//... Ports für Diagnose LEDs:....................
constexpr uint8_t LN_FAILED(12);       // bei LN-Ausfall
constexpr uint8_t FC_NO_TELEGRAMM(13); // bisher kein Telegramm erhalten
constexpr uint8_t FC_RATE_1_1(A1);     // Takt = 1:1
constexpr uint8_t FC_RATE_1_X(A2);     // Takt > 1:1
constexpr uint8_t FC_STOPPED(A3);      // Takt angehalten
