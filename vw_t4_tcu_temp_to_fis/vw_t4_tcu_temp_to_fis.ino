/*
  VW T4 ATF temp on FIS display

  Using OBD protocol KW1281 for getting the ATF temp from the Transmission Control Unit (TCU)
  Using 3LB protocol to show info on FIS-display

  Credits:

  This project would not have been possible without the help of other open source projects out there.
  Credits and thanks goes to:

  Alexander's software and OBD2-interface
  
  http://grauonline.de/wordpress/?p=74
  docs: // https://www.blafusel.de/obd/obd2_kw1281.html

  NOTE: For the level shifting, I used a 'AutoDia K409 Profi USB adapter', disassembled it,
      and connected the Arduino to the level shifter chip (LM339) - the original FTDI chip TX line
      was removed (so it does not influence the communication)

  And to these projects for information for communication with the FIS-display
    Thanks for publishing the code for the 1999 Audi
    https://github.com/tomaskovacik/arduino/tree/master/VWFIS

    Thanks for the code to understand the 3LB protocl. This is really a great project!
    https://github.com/pohlinkzei/RPi_MFD_Interface
    Thanks for the hints for checksum generation.
    I managed to figure out the 2002 VW checksum-generation after study this page and numbercrunching in LibreOffice-Calc :-)
    http://www.gti-tdi.de/board4/index.php?thread/3787-tacho-full-fis-displaing-3lb/
*/

// Enables debug-prints to Serial
#define DEBUG_PRINT 1

// Enables output to FIS-display
#define ENABLE_FIS 1

// Enables printing of Memory usage
// Requiers library MemoryFree.h
//#define DEBUG_MEM 1


#include "VWOBD2.h"
#ifdef ENABLE_FIS
#include "VW2002FISWriter.h"
#endif
#ifdef DEBUG_MEM
#include <MemoryFree.h>
#endif

// OBD2
#define pinKLineRX 2
#define pinKLineTX 3
VWOBD2 vwobd2(pinKLineRX, pinKLineTX); // RX, TX

// FIS
#define FIS_CLK 13  // - Arduino 13 - PB5
#define FIS_DATA 11 // - Arduino 11 - PB3
#define FIS_ENA 8   // - Arduino 8 - PB0
VW2002FISWriter fisWriter( FIS_CLK, FIS_DATA, FIS_ENA );



#if DEBUG_MEM
void printFreeMem() {
#ifdef DEBUG_PRINT
  Serial.print(F("freeMemory()="));
  Serial.println(freeMemory());
#endif
}
#endif


/*
 * 
 * setup()
 * 
 */
void setup() {
#ifdef DEBUG_PRINT
  //  Serial.begin(115200);
  Serial.begin(250000);
  Serial.println(F("SETUP"));
#endif

#ifdef ENABLE_FIS
#ifdef DEBUG_PRINT
  Serial.println("Init FIS-Writer");
#endif
  fisWriter.FIS_init();
#endif

#ifdef DEBUG_PRINT
  Serial.println(F("START"));
#endif

#ifdef ENABLE_FIS
//  fisWriter.sendMsg(F("STARTING"), "", true);
#endif
} // setup



/*
 * 
 * loop ()
 * 
 */
#ifdef DEBUG_PRINT
int loop_counter = 0;
#endif
char heartbeatChar = ' ';
static char fisBuffer[17] = {'A', 'T', 'F' , ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '} ;
static char engagedGearBuffer[2] = {' ', ' '};

void loop() {
#ifdef DEBUG_PRINT
  Serial.print(F("loop : "));
  Serial.println(loop_counter);
  loop_counter++;
#if DEBUG_MEM
  printFreeMem();
#endif
#endif

  if (vwobd2.getCurrAddr() != ADR_Gears) { // ADR_Radio ADR_Gears
#ifdef DEBUG_PRINT
    Serial.println(F("loop - connecting Gears"));
#endif
    vwobd2.connect(ADR_Gears, 1200);
  } else {
#ifdef DEBUG_PRINT
    Serial.println(F("loop - readSensors"));
#endif
    vwobd2.readSensors(5);

    // get engaged gear
    vwobd2.convertGear(engagedGearBuffer);

    // toggle heartbeat
    heartbeatChar = (heartbeatChar=='-') ? 28 : '-'; // 28 means align to left

    // display [ATF    -]
    //         [1H 007 C]
    sprintf(fisBuffer, "ATF    %c%s %3i C", heartbeatChar,engagedGearBuffer,vwobd2.gearOilTemp);

    // Write to FIS-display
#ifdef ENABLE_FIS
    fisWriter.sendMsg(fisBuffer);
#endif

  } // if connected
} // loop

