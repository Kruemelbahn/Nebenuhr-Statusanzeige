/*
 * Nebenuhr-Statusanzeige-WS2812B
 
used I2C-Addresses:
  - 0x20 LCD-Panel
  
discrete In/Outs used for functionalities:
  -  0    (used  USB)
  -  1    (used  USB)
  -  2 not used
  -  3 not used
	-  4 not used
	-  5 not used
  -  6 Out used   by HeartBeat
  -  7 Out used   WS2812B Dataline
  -  8 not used
  -  9 not used
  - 10 not used
  - 11 not used
  - 12 IN  used   LN-Ausfall
  - 13 IN  used   bisher kein Telegramm erhalten
  - 14 IN  used   LN-Watcher
  - 15 IN  used   Takt = 1:1
  - 16 IN  used   Takt > 1:1
  - 17 IN  used   Takt angehalten
  - 18     (used by I²C: SDA)
  - 19     (used by I²C: SCL)

 *************************************************** 
 *  Copyright (c) 2026 Michael Zimmermann <https://kruemelsoft.hier-im-netz.de>
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

//=============================================
#if defined(__AVR_ATmega8__)
#endif
#if defined(__AVR_ATmega328P__)
  // dann ist der Flash groß genug :-)
  #define LCD     // used in GlobalOutPrint.ino
#endif

//=============================================

constexpr uint16_t NUMPIXELS(8);

#include "CV.h"

#define MANUFACTURER_ID  13   // NMRA: DIY
#define DEVELOPER_ID  29      // NMRA: my ID, (1 = FREMO, see https://groups.io/g/LocoNet-Hackers/files/LocoNet%20Hackers%20DeveloperId%20List_v29.html)

//=== declaration of var's =======================================

#include <HeartBeat.h>
HeartBeat oHeartbeat;

//========================================================
void setup()
{
#if defined DEBUG
  // initialize serial and wait for port to open:
  Serial.begin(57600);
#endif
  ReadCVsFromEEPROM();
  
#if defined LCD
  CheckAndInitLCDPanel();
#endif

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
#if defined LCD
  HandleLCDPanel();
#endif

  //=== do Clock handling ===
  HandleClockHandling();
}
