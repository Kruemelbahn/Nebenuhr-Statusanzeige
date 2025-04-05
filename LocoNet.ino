//=== LocoNet for Nebenuhr-Statusanzeige ===
#include <LocoNet.h>

//=== declaration of var's =======================================
LocoNetFastClockClass  FastClock;

static  lnMsg          *LnPacket;
static  LnBuf          LnTxBuffer;

unsigned long ul_LastFastClockTick = 0;

/*
*  https://www.digitrax.com/static/apps/cms/media/documents/loconet/loconetpersonaledition.pdf
*  https://wiki.rocrail.net/doku.php?id=loconet:ln-pe-de
*  http://embeddedloconet.sourceforge.net/SV_Programming_Messages_v13_PE.pdf
*  remark: this software didnot follow '2.2.6) Standard SV/EEPROM Locations'
*/
// default OPC's are defined in 'utility\ln_opc.h'
// A3 is already used  and defined as OPC_LOCO_F912
// A8 is already used by FRED  and defined as OPC_DEBUG
// A8 is already used by FREDI and defined as OPC_FRED_BUTTON
//#define OPC_FRED_BUTTON     0xA8
// AF is already used by FREDI and defined as OPC_SELFTEST resp. as OPC_FRED_ADC
// ED is already used  and defined as OPC_IMM_PACKET
// EE is already used  and defined as OPC_IMM_PACKET_2
#define SV2_Format_1	0x01
#define SV2_Format_2	0x02

#define PIN_TX   7
#define PIN_RX   8

uint8_t ui8_WaitForTelegram = 0;
unsigned long ul_WaitStartForTelegram = 0;
int16_t i16_FracMinStart(0);

//=== functions for receiving telegrams from SerialMonitor =======
#if defined TELEGRAM_FROM_SERIAL
static const int MAX_LEN_LNBUF = 64;
uint8_t ui8_PointToBuffer = 0;

uint8_t ui8a_receiveBuffer[MAX_LEN_LNBUF];
uint8_t ui8a_resultBuffer[MAX_LEN_LNBUF];

void ClearReceiveBuffer()
{
  ui8_PointToBuffer = 0;
  for(uint8_t i = 0; i < MAX_LEN_LNBUF; i++)
    ui8a_receiveBuffer[i] = 0;
}

uint8_t Adjust(uint8_t ui8_In)
{
  uint8_t i = ui8_In;
  if((i >= 48) && (i <= 57))
    i -= 48;
  else
    if((i >= 65) && (i <= 70))
      i -= 55;
    else
      if((i >= 97) && (i <= 102))
        i -= 87;
      else
        i = 0;
  return i;
}

uint8_t AnalyzeBuffer()
{
  uint8_t i = 0;
  uint8_t ui8_resBufSize = 0;
  while(uint8_t j = ui8a_receiveBuffer[i++])
  {
    if(j != ' ')
    {
      j = Adjust(j);
      uint8_t k = ui8a_receiveBuffer[i++];
      if(k != ' ')
        ui8a_resultBuffer[ui8_resBufSize++] = j * 16 + Adjust(k);
      else
        ui8a_resultBuffer[ui8_resBufSize++] = j;
    }
  }

  // add checksum:
  uint8_t ui8_checkSum(0xFF);
  for(i = 0; i < ui8_resBufSize; i++)
    ui8_checkSum ^= ui8a_resultBuffer[i]; 
  bitWrite(ui8_checkSum, 7, 0);     // set MSB zero
  ui8a_resultBuffer[ui8_resBufSize++] = ui8_checkSum;

  return ui8_resBufSize;
}

#endif

uint8_t GetWaitForTelegram() { return ui8_WaitForTelegram; }
void SetWaitForTelegram(uint8_t iType)
{
  ui8_WaitForTelegram = iType;
  ul_WaitStartForTelegram = millis();
}

//=== functions ==================================================
void InitLocoNet()
{
  // First initialize the LocoNet interface, specifying the TX Pin
  LocoNet.init(PIN_TX);

  if(ENABLE_LN_FC_MODUL)
  {
    // Initialize the Fast Clock
    FastClock.init(0, 0, 1); // DCS100CompatibleSpeed - CorrectDCS100Clock - NotifyFracMin
  
    // Poll the Current Time from the Command Station
    PollFastClock();
  }

#if defined TELEGRAM_FROM_SERIAL
  ClearReceiveBuffer();
#endif
}

void HandleLocoNetMessages()
{
#if defined TELEGRAM_FROM_SERIAL
  LnPacket = NULL;
  if (Serial.available())
  {
    byte byte_Read = Serial.read();
    if(byte_Read == '\n')
    {
      if(AnalyzeBuffer() > 0)
        LnPacket = (lnMsg*) (&ui8a_resultBuffer);
      ClearReceiveBuffer();
    }
    else
    {
      if(ui8_PointToBuffer < MAX_LEN_LNBUF)
        ui8a_receiveBuffer[ui8_PointToBuffer++] = byte_Read;
    }
  }
#else
  // Check for any received LocoNet packets
  LnPacket = LocoNet.receive();
#endif
  if(LnPacket)
  {
    if (ENABLE_LN_FC_MODUL)
    {
      // JMRI sends only EF 0E 7B ... (OPC_WR_SL_DATA) with clk_cntrl == 0 -> so we correct it:
      if (ENABLE_LN_FC_JMRI && (LnPacket->fc.slot == FC_SLOT) && (LnPacket->fc.command == OPC_WR_SL_DATA) && !(LnPacket->fc.clk_cntrl & 0x40))
        LnPacket->fc.clk_cntrl |= 0x40;
      if ((LnPacket->fc.slot == FC_SLOT) && ((LnPacket->fc.command == OPC_WR_SL_DATA) || (LnPacket->fc.command == OPC_SL_RD_DATA)))
        i16_FracMinStart = 0x3FFF/*FC_FRAC_MIN_BASE*/ - ((LnPacket->fc.frac_minsh << 7) + LnPacket->fc.frac_minsl);
      FastClock.processMessage(LnPacket); // will call 'notifyFastClock' with sync=1 if neccessary 
    }

    uint8_t ui8_msgLen = getLnMsgSize(LnPacket);
    uint8_t ui8_msgOPCODE = LnPacket->data[0];
    //====================================================
    if (ui8_msgLen == 16)
    {
      if (ui8_msgOPCODE == OPC_PEER_XFER) // E5
        HandleE5MessageFormat2();
    } // if(ui8_msgLen == 16)
    //====================================================
  } // if(LnPacket)

	if (ENABLE_LN_FC_MODUL/* we are also Slave */ && isTimeForProcessActions(&ul_LastFastClockTick, 67))
    FastClock.process66msActions(); // will call 'notifyFastClockFracMins' with sync=0 if neccessary (and so clock will continue running internally)
}  

void HandleFracMins(uint16_t FracMins)
{
  if (!ENABLE_LN_FC_MODUL || !GetClockRate())
    return;

  if ((i16_FracMinStart - 910) > FracMins) // 910 = (approx.) 60000ms / 66ms
  {
    i16_FracMinStart = FracMins;
    if(ENABLE_LN_FC_INTERN)
      IncFastClock(1);
  }
}

LN_STATUS PollFastClock()
{
  // Poll the current time from the command station
  // ...we don't use "FastClock.poll();" because it has no return (so we don't see any error and can't react to the result)...
#if defined DEBUG
  Serial.println("...poll FastClock...");
#endif
  return LocoNet.send(OPC_RQ_SL_DATA, FC_SLOT, 0);
}

#if defined DEBUG || defined TELEGRAM_FROM_SERIAL
void Printout(char ch)
{
  if(LnPacket)
  {
    // print out the packet in HEX
    Serial.print(ch);
    Serial.print(F("X: "));
    uint8_t ui8_msgLen = getLnMsgSize(LnPacket); 
    for (uint8_t i = 0; i < ui8_msgLen; i++)
    {
      uint8_t ui8_val = LnPacket->data[i];
      // Print a leading 0 if less than 16 to make 2 HEX digits
      if(ui8_val < 16)
        Serial.print('0');
      Serial.print(ui8_val, HEX);
      Serial.print(' ');
    }
    Serial.println();
  } // if(LnPacket)
}
#endif
