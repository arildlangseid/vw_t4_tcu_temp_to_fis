#include "VW2002FISWriter.h"
#include <MemoryFree.h>
#include <Arduino.h>

// #define ENABLE_IRQ 1
// #define DEBUG_MEM 1


/**
   Constructor
*/
VW2002FISWriter::VW2002FISWriter(uint8_t clkPin, uint8_t dataPin, uint8_t enaPin)
{
  _FIS_WRITE_CLK = clkPin;
  _FIS_WRITE_DATA = dataPin;
  _FIS_WRITE_ENA = enaPin;

}

/**
   Destructor
*/
VW2002FISWriter::~VW2002FISWriter()
{
}

/**

   Initialize instrument-cluster

*/
void VW2002FISWriter::FIS_init() {
  // set port signals
  pinMode(_FIS_WRITE_ENA, OUTPUT);
  digitalWrite(_FIS_WRITE_ENA, LOW);
  FIS_WRITE_stopENA();
  pinMode(_FIS_WRITE_CLK, OUTPUT);
  digitalWrite(_FIS_WRITE_CLK, HIGH);
  pinMode(_FIS_WRITE_DATA, OUTPUT);
  digitalWrite(_FIS_WRITE_DATA, HIGH);

  // init port
  FIS_WRITE_send_3LB_singleByteCommand(0xF0);
  delay(200);
  FIS_WRITE_send_3LB_singleByteCommand(0xC3);
  delay(200);
  FIS_WRITE_send_3LB_singleByteCommand(0xF0);
  delay(200);
  FIS_WRITE_send_3LB_singleByteCommand(0xC3);
  delay(200);
}; // FIS_init

static char tx_array[20];

#ifdef USE_STRINGS
void VW2002FISWriter::sendMsg(String line1, String line2, bool center) {
  // char msg1[] = {0x81, 18, 240, ' ', 'N', 'R', 'K', ' ', 'P', '1', ' ', 'F', 'M', '1', '.', '1', ' ', ' ', 28, 75};

  //Serial.println("d");

  // fill lines to 8 chars
  while (line1.length() < 8) line1 += " ";
  while (line2.length() < 8) line2 += " ";
  /*
    Serial.print("[");
    Serial.print(line1);
    Serial.print("]");
    Serial.print("[");
    Serial.print(line2);
    Serial.println("]");
    Serial.println("f");
  */
  // build tx_array
  tx_array[0] = 0x81; // command to set text-display in FIS
  tx_array[1] = 18; // Length of this message (command and this length not counted
  tx_array[2] = 240; // unsure what this is

  line1.toCharArray(&tx_array[3], 9);
  if (tx_array[10] == 32 && !center) tx_array[10] = 28; // set last char to 28 if not center
  line2.toCharArray(&tx_array[11], 9);
  if (tx_array[18] == 32 && !center) tx_array[18] = 28; // set last char to 28 if not center
  tx_array[19] = (char)checksum((uint8_t*)tx_array);

  sendRawMsg(tx_array);

}
#endif

void VW2002FISWriter::sendMsg(char msg[]) {
  // build tx_array
  tx_array[0] = 0x81; // command to set text-display in FIS
  tx_array[1] = 18; // Length of this message (command and this length not counted
  tx_array[2] = 240; // unsure what this is

  for (int i = 0; i < 16; i++) { // TODO: use memcpy
    tx_array[3 + i] = msg[i];
  }
  tx_array[19] = (char)checksum((uint8_t*)tx_array);

  sendRawMsg(tx_array);
}

/**

   Prepare and send Text-Message

*/
void VW2002FISWriter::sendRawMsg(char in_msg[]) {
  FIS_WRITE_send_3LB_msg(in_msg);
}

/**

   Send Text-Message out on 3LB port to instrument cluster

*/
void VW2002FISWriter::FIS_WRITE_send_3LB_msg(char in_msg[]) {
  uint16_t timeout_us;

#ifdef ENABLE_IRQ
  cli();
#endif

  sendEnablePulse();

  // Send FIS-command
  FIS_WRITE_3LB_sendByte(in_msg[FIS_MSG_COMMAND]);
  setDataHigh();

  byte msg_length = in_msg[FIS_MSG_LENGTH];
  byte msg_end = msg_length + 1;

  // Step 2 - wait for response from cluster to set ENA-High
  timeout_us = 1500;
  while (!digitalRead(_FIS_WRITE_ENA) && timeout_us > 0) {
    delayMicroseconds(20);
    timeout_us -= 20;
  }

  uint8_t crc = 0;
  for (int i = 1; i <= msg_end; i++)
  {
    // Step 9.2 - ENA-Low detected
    if (i > 1) {
      setDataLow();
    }
    delayMicroseconds(40);

    // calculate checksum
    if (i == msg_end) {
      crc ^= 147;
      crc --;
      in_msg[i] = crc;
    } else if (i != 1) {
      crc ^= in_msg[i];
    }

    FIS_WRITE_3LB_sendByte(in_msg[i]);

    setDataLow();

    // Step 10.2 - wait for response from cluster to set ENA-High
    timeout_us = 1500;
    while (!digitalRead(_FIS_WRITE_ENA) && timeout_us > 0) {
      delayMicroseconds(1);
      timeout_us -= 1;
    }
  }
  // Step 9.5 - ENA-Low detected
  setDataLow();

#ifdef ENABLE_IRQ
  sei();
#endif
}

/**

   Send Keep-Alive message

*/
void VW2002FISWriter::sendKeepAliveMsg() {
  delay(100);
  FIS_WRITE_send_3LB_singleByteCommand(0xC3);
  delay(100);
}

void VW2002FISWriter::displayOff() {
  sendRawMsg((char*)off);
}
void VW2002FISWriter::displayBlank() {
  sendRawMsg((char*)blank);
}

/**

   Send Single-Command out on 3LB port to instrument cluster

*/
void VW2002FISWriter::FIS_WRITE_send_3LB_singleByteCommand(uint8_t txByte) {

#ifdef ENABLE_IRQ
  cli();
#endif

  sendEnablePulse();

  // Send FIS-command
  FIS_WRITE_3LB_sendByte(txByte);

  delayMicroseconds(30);
  setDataLow();

#ifdef ENABLE_IRQ
  sei();
#endif
}

/**

   Send 3LB Enable pulse

*/
void VW2002FISWriter::sendEnablePulse() {
  FIS_WRITE_startENA();
  delayMicroseconds(41);
  FIS_WRITE_stopENA();
  delayMicroseconds(37);
} // sendEnablePulse


void VW2002FISWriter::printFreeMem() {
  Serial.print(F("FIS:freeMemory()="));
  Serial.println(freeMemory());
}

/**

   Send byte out on 3LB port to instrument cluster

*/
void VW2002FISWriter::FIS_WRITE_3LB_sendByte(int in_byte) {

  int tx_byte = 0xff - in_byte;
  for (int i = 7; i >= 0; i--) {
    setClockLow();

    switch ((tx_byte & (1 << i)) > 0 ) {
      case 1: setDataHigh();
        break;
      case 0: setDataLow();
        break;
    }
    delayMicroseconds(FIS_WRITE_PULSEW / 1);
    setClockHigh();
    delayMicroseconds(FIS_WRITE_PULSEW * 2);
  }
}

/**
   Set 3LB ENA active High
*/
void VW2002FISWriter::FIS_WRITE_startENA() {
  pinMode(_FIS_WRITE_ENA, OUTPUT);
  digitalWrite(_FIS_WRITE_ENA, HIGH);
}
/**
   Set 3LB ENA paaive Low
*/
void VW2002FISWriter::FIS_WRITE_stopENA() {
  digitalWrite(_FIS_WRITE_ENA, LOW);
  pinMode(_FIS_WRITE_ENA, INPUT);
}

/**
   Set 3LB CLK High
*/
void VW2002FISWriter::setClockHigh() {
  PORT_3LB |= (1 << CLK);
}
/**
   Set 3LB CLK Low
*/
void VW2002FISWriter::setClockLow() {
  PORT_3LB &= ~(1 << CLK);
}
/**
   Set 3LB DATA High
*/
void VW2002FISWriter::setDataHigh() {
  PORT_3LB |= (1 << DATA);
}
/**
   Set 3LB DATA Low
*/
void VW2002FISWriter::setDataLow() {
  PORT_3LB &= ~(1 << DATA);
}

/**
   checksum routine to calculate the crc

   takes komplete message as parameter including messagelength and the 240 constant followed by the 16 byte message
   example: uint8_t msg[] = {18, 240, 32, 78, 82, 75, 32, 80, 49, 32, 70, 77, 49, 46, 49, 32, 32, 28};

*/
uint8_t VW2002FISWriter::checksum( volatile uint8_t in_msg[]) {
  uint8_t crc = in_msg[0];
  for (int i = 1; i < 17; i++)
  {
    crc ^= in_msg[i];
  }
  // just obey the next lines of code....
  crc ^= 147;
  crc -= 1;

  return crc;
}

