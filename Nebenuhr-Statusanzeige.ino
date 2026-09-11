/*
 * Nebenuhr-Statusanzeige
 
used I2C-Addresses:
  - 0x20 LCD-Panel
  
discrete In/Outs used for functionalities:
  -  0    (used  USB)
  -  1    (used  USB)
  -  2 not used
  -  3 Out used   BLUE
	-  4 Out used   CLK TM1637 (optional)
	-  5 Out used   DIO TM1637 (optional)
  -  6 Out used   by HeartBeat
  -  7 Out used   by LocoNet [TxD]
  -  8 In  used   by LocoNet [RxD]
  -  9 Out used   RED
  - 10 Out used   WS2812B Dataline
  - 11 Out used   GREEN
  - 12 OUT used   diskrete LED rt für Diagnose: blinkt bei LN-Ausfall
  - 13 OUT used   diskrete LED gb für Diagnose: bisher kein Telegramm erhalten
  - 14 IN  used   LN-Watcher
  - 15 OUT used   diskrete LED ws für Diagnose: Takt = 1:1
  - 16 OUT used   diskrete LED gn für Diagnose: Takt > 1:1
  - 17 OUT used   diskrete LED rt für Diagnose: Takt angehalten
  - 18     (used by I²C: SDA)
  - 19     (used by I²C: SCL)

 *************************************************** 
 *  Copyright (c) 2025 Michael Zimmermann <https://kruemelsoft.hier-im-netz.de>
 *  All rights reserved.
 *
 *  LICENSE
 *  -------
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************
 */

//=== global stuff =======================================
//#define DEBUG 1   // enables Outputs (debugging informations) to Serial monitor
                  // note: activating SerialMonitor in Arduino-IDE
                  //       will create a reset in software on board!

//#define DEBUG_CV 1  // enables CV-Output to serial port during program start (saves 180Bytes of code :-)
//#define DEBUG_MEM 1 // enables memory status on serial port (saves 350Bytes of code :-)

//#define FAST_CLOCK_LOCAL 1  //uses local ports 2 and 3 as slaveclockmodule as an alternative/additional for local I2C-Module
                              // not usable with this hardware, ports already used for LED-Stripe

//#define TELEGRAM_FROM_SERIAL 1  // enables receiving telegrams from SerialMonitor
                                // instead from LocoNet-Port (which is inactive then)

#define LCD     // used in GlobalOutPrint.ino

constexpr uint16_t NUMPIXELS(8);

#include "CV.h"

#define ENABLE_LN             (1)
#define ENABLE_LN_E5          (1)
#define ENABLE_LN_FC_MODUL    (1)
#define ENABLE_LN_FC_INTERN   (GetCV(ADD_FUNCTIONS_1) & 0x08)
#define ENABLE_LN_FC_INVERT   (GetCV(ADD_FUNCTIONS_1) & 0x20)
#define ENABLE_LN_FC_SLAVE    (ENABLE_LN_FC_MODUL)
#define ENABLE_LN_FC_JMRI     (GetCV(ADD_FUNCTIONS_1) & 0x10)

#define UNREFERENCED_PARAMETER(P) { (P) = (P); }

#define MANUFACTURER_ID  13   // NMRA: DIY
#define DEVELOPER_ID  58      // NMRA: my ID, should be > 28 (1 = FREMO, see https://groups.io/g/LocoNet-Hackers/files/LocoNet%20Hackers%20DeveloperId%20List_v27.html)

//=== declaration of var's =======================================

#include <LocoNet.h>    // requested for notifyFastClock

#include <HeartBeat.h>
HeartBeat oHeartbeat;

static const uint8_t MIN_COLOR (50); // rot - grün - blau - weiß - gelb
static const uint8_t MAX_COLOR (54); // rot - grün - blau - weiß - gelb

//========================================================
void setup()
{
#if defined DEBUG || defined TELEGRAM_FROM_SERIAL
  // initialize serial and wait for port to open:
  Serial.begin(57600);
#endif

  ReadCVsFromEEPROM();
  
  CheckAndInitLCDPanel();

  InitLocoNet();
  
  InitFastClock();
	
  InitClockHandling();
}

void loop()
{
  // light the Heartbeat LED
  oHeartbeat.beat();
  // generate blinken
  Blinken();

  //=== do LCD handling ==============
  // can be connected every time
  // panel only necessary for setup CV's (or some status informations):
  HandleLCDPanel();

  //=== do LocoNet handling ==========
  HandleLocoNetMessages();

	//=== do FastClock handling ===
	HandleFastClock();

  //=== do Clock handling ===
  HandleClockHandling();
	
#if defined DEBUG
  #if defined DEBUG_MEM
    ViewFreeMemory();  // shows memory usage
    ShowTimeDiff();    // shows time for 1 cycle
  #endif
#endif
}

void StopClock() { ; }  // copy from 'Uhrenzentrale - ClockCommand.ino', for compatibility; do nothing here...

/*=== will be called from LocoNetFastClockClass
			if telegram is OPC_SL_RD_DATA [0xE7] or OPC_WR_SL_DATA [0xEF] and clk-state != IDLE ==================
  Rate: 0 = Freeze clock, 1 = normal, 10 = 10:1 etc. max is 0x7F 
*/
void notifyFastClock( uint8_t Rate, uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t Sync )
{
#if defined DEBUG
  Serial.println();
  Serial.print(F("notifyFastClock "));
  Serial.print(Hour);
  Serial.print(":");
  decout(Serial, Minute, 2);
  Serial.print("[Rate=");
  Serial.print(Rate);
  Serial.print("][Sync=");
  Serial.print(Sync);
	Serial.println("]");
#endif    
  SetFastClock(Rate, Day, Hour, Minute, Sync);
}

void notifyFastClockFracMins(uint16_t FracMins)
{
  HandleFracMins(FracMins);
}
