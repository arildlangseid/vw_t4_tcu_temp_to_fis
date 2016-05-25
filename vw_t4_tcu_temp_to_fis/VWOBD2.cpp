#include "VWOBD2.h"
#include <Arduino.h>


//#define DEBUG_DEEP 1
#define DEBUG_PRINT_ERROR 1

/*



   Pubblic memberfunctions



*/

/*
   Constructor
*/
VWOBD2::VWOBD2(uint8_t receivePin, uint8_t transmitPin)
{
  _OBD_RX_PIN = receivePin;
  _OBD_TX_PIN = transmitPin;

  pinMode(transmitPin, OUTPUT);
  digitalWrite(transmitPin, HIGH);

  obd = new NewSoftwareSerial(receivePin, transmitPin, false); // RX, TX, inverse logic

#ifdef DEBUG_PRINT_ERROR
  Serial.println(F("VWOBD2 created"));
#endif
}

/*
   Destructor
*/
VWOBD2::~VWOBD2()
{
  delete obd;
  obd = NULL;
}

/*

   Connect ()

*/
bool VWOBD2::connect(uint8_t addr, int baudrate) {
#ifdef DEBUG_DEEP
  Serial.print(F("------connect addr="));
  Serial.print(addr);
  Serial.print(F(" baud="));
  Serial.println(baudrate);
#endif

  blockCounter = 0;
  currAddr = 0;
  obd->begin(baudrate);
  KWP5BaudInit(addr);
  // answer: 0x55, 0x01, 0x8A
  char cs[3];
  int size = 3;
  if (!KWPReceiveBlock(cs, 3, size, true)) return false;
  if (    (((uint8_t)cs[0]) != 0x55)            //55
          ||   (((uint8_t)cs[1]) != 0x01)       //01
          ||   (((uint8_t)cs[2]) != 0x8A)   ) { //8A
#ifdef DEBUG_PRINT_ERROR
    Serial.println(F("ERROR: invalid magic"));
#endif
    disconnect();
    errorData++;
    return false;
  }
  /*
    char a[2];
    size = 1;
    if (!KWPReceiveBlock(s, 3, size)) return false;
    Serial.print("Address:");
    a[1]=0;
    Serial.println(a[0],HEX);
  */

  currAddr = addr;
  connected = true;
  if (!readConnectBlocks()) return false;
  return true;
}

/*
   Disconnect()
*/
void VWOBD2::disconnect() {
  connected = false;
  currAddr = 0;
}


static byte atfTemp = 0;
static byte atfMultifuncSwitch = 0;
static byte atfGearEngaged = 0;
static byte atfEngineRPM = 0;
static char rs[128];
bool VWOBD2::readSensors(int group) {
#ifdef DEBUG_DEEP
  Serial.print(F("-- readSensors meassureBlock="));
  Serial.println(group);
#endif

  sprintf(rs, "\x04%c\x29%c\x03", blockCounter, group);

  if (!KWPSendBlock(rs, 5)) return false;
  int size = 0;
  KWPReceiveBlock(rs, 128, size);

  if (rs[2] == '\x02') {
#ifdef DEBUG_DEEP
    Serial.println(F("readConnectBlocks: got 02"));
#endif
    return true;
  } else if (rs[2] != '\xf4') {
#ifdef DEBUG_PRINT_ERROR
    Serial.println(F("ERROR: invalid answer"));
#endif
    disconnect();
    errorData++;
    return false;
  }

  atfTemp = rs[3];
  atfMultifuncSwitch = rs[4];
  atfGearEngaged = rs[5];
  atfEngineRPM = rs[6];

  gearOilTemp = convertATFtempToC(atfTemp);
  gearEngaged = atfGearEngaged;

  return true;
}

bool VWOBD2::isConnected() {
  return connected;
}
uint8_t VWOBD2::getCurrAddr() {
  return currAddr;
}

uint8_t VWOBD2::convertATFtempToC(int atfTemp) {
  switch (atfTemp) {
    case 0: return 0;
    case 1: return 3;
    case 2: return 7; // verified
    case 3: return 10;
    case 4: return 14;
    case 5: return 15;
    case 6: return 18;
    case 7: return 22; // verified
    case 8: return 29; // verified
    case 9: return 31;
    case 10: return 33;
    case 11: return 36;
    case 12: return 40;
    case 13: return 44;
    case 14: return 47;
    case 15: return 51;
    case 16: return 55;
    case 17: return 59;
    case 18: return 60;
    case 19: return 61;
    case 20: return 62;
    case 21: return 64;
    case 22: return 65;
    case 23: return 66;
    case 24: return 68;
    case 25: return 69;
    case 26: return 70;
    case 27: return 72;
    case 28: return 74;
    case 29: return 75;
    case 30: return 76;
    case 31: return 78;
    case 32: return 80;
    case 33: return 81;
    case 34: return 82;
    case 35: return 83;
    case 36: return 84;
    case 37: return 85;
    case 38: return 86;
    case 39: return 87;
    case 40: return 88;
    case 41: return 89;
    case 42: return 90;
    case 43: return 91;
    case 44: return 92;
    case 45: return 93;
    case 46: return 94;
    case 47: return 95;
    case 48: return 96;
    case 49: return 97;
    case 50: return 98;
    case 51: return 99;
    case 52: return 100;
    case 53: return 101;
    case 54: return 102;
    case 55: return 103;
    case 56: return 104;
    case 57: return 105;
    case 58: return 106;
    case 59: return 107;
    case 60: return 108;
    case 61: return 109;
    case 62: return 110;
    default: return atfTemp;
  }
}

void VWOBD2::convertGear(char buffer[2]) {
  switch (gearEngaged) {
    case 0: buffer[0] = 'P'; buffer[1] = ' '; break;  // 0000
    case 1: buffer[0] = 'R'; buffer[1] = ' '; break;  // 0001
    case 2: buffer[0] = 'N'; buffer[1] = ' '; break;  // 0010
    case 3: buffer[0] = '1'; buffer[1] = 'H'; break;  // 0011
    case 4: buffer[0] = '2'; buffer[1] = 'H'; break;  // 0100
    case 5: buffer[0] = '3'; buffer[1] = 'H'; break;  // 0101
    case 6: buffer[0] = '4'; buffer[1] = 'H'; break;  // 0110
    case 7: buffer[0] = '7'; buffer[1] = ' '; break;  // 0111
    case 8: buffer[0] = '8'; buffer[1] = ' '; break;  // 1000
    case 9: buffer[0] = '3'; buffer[1] = 'M'; break;  // 1001
    case 10: buffer[0] = '4'; buffer[1] = 'M'; break; // 1010
    case 11: buffer[0] = '1'; buffer[1] = '1'; break; // 1011
    default: buffer[0] = '-'; buffer[1] = '-'; break;
  }
}


/**



   Private memberfunctions



*/

void VWOBD2::obdWrite(uint8_t data) {
  /*
    #ifdef DEBUG
    Serial.print("uC:");
    Serial.println(data, HEX);
    #endif
  */
  delay(3);
  obd->write(data);
}

uint8_t VWOBD2::obdRead() {
  unsigned long timeout = millis() + 1000;
  while (!obd->available()) {
    if (millis() >= timeout) {
#ifdef DEBUG_PRINT_ERROR
      Serial.println(F("ERROR: obdRead timeout"));
#endif
      disconnect();
      errorTimeout++;
      return 0;
    }
  }
  uint8_t data = obd->read();
  /*
    #ifdef DEBUG
    Serial.print("ECU:");
    Serial.println(data, HEX);
    #endif
  */
  return data;
}


// 5Bd, 7O1
void VWOBD2::send5baud(uint8_t data) {
  // // 1 start bit, 7 data bits, 1 parity, 1 stop bit
#define bitcount 10
  byte bits[bitcount];
  byte even = 1;
  byte bit;
  for (int i = 0; i < bitcount; i++) {
    bit = 0;
    if (i == 0)  bit = 0;
    else if (i == 8) bit = even; // computes parity bit
    else if (i == 9) bit = 1;
    else {
      bit = (byte) ((data & (1 << (i - 1))) != 0);
      even = even ^ bit;
    }
#ifdef DEBUG_DEEP
    Serial.print(F("bit"));
    Serial.print(i);
    Serial.print(F("="));
    Serial.print(bit);
    if (i == 0) Serial.print(F(" startbit"));
    else if (i == 8) Serial.print(F(" parity"));
    else if (i == 9) Serial.print(F(" stopbit"));
    Serial.println();
#endif
    bits[i] = bit;
  }
  // now send bit stream
  for (int i = 0; i < bitcount + 1; i++) {
    if (i != 0) {
      // wait 200 ms (=5 baud), adjusted by latency correction
      delay(200);
      if (i == bitcount) break;
    }
    if (bits[i] == 1) {
      // high
      digitalWrite(_OBD_TX_PIN, HIGH);
    } else {
      // low
      digitalWrite(_OBD_TX_PIN, LOW);
    }
  }
  obd->flush();
}


bool VWOBD2::KWP5BaudInit(uint8_t addr) {
#ifdef DEBUG_DEEP
  Serial.println(F("---KWP 5 baud init"));
#endif

  send5baud(addr);
  return true;
}


bool VWOBD2::KWPSendBlock(char *s, int size) {
#ifdef DEBUG_DEEP
  Serial.print(F("---KWPSend sz="));
  Serial.print(size);
  Serial.print(F(" blockCounter="));
  Serial.println(blockCounter);
  // show data
  Serial.print(F("OUT:"));
  for (int i = 0; i < size; i++) {
    uint8_t data = s[i];
    Serial.print(data, HEX);
    Serial.print(F(" "));
  }
  Serial.println();
#endif

  for (int i = 0; i < size; i++) {
    uint8_t data = s[i];
    obdWrite(data);
    if (i < size - 1) {
      uint8_t complement = obdRead();
      if (complement != (data ^ 0xFF)) {
#ifdef DEBUG_PRINT_ERROR
        Serial.println(F("ERROR: invalid complement"));
#endif
        disconnect();
        errorData++;
        return false;
      }
    }
  }
  blockCounter++;
  return true;
}



/*

   KWPReceiveBlock

*/
// count: if zero given, first received byte contains block length
// 4800, 9600 oder 10400 Baud, 8N1
bool VWOBD2::KWPReceiveBlock(char s[], int maxsize, int &size, bool init_delay) {
  bool ackeachbyte = false;
  uint8_t data = 0;
  int recvcount = 0;

  if (size == 0) ackeachbyte = true;

#ifdef DEBUG_DEEP
  Serial.print(F("---KWPReceiveBlock sz="));
  Serial.print(size);
  Serial.print(F(" blockCounter="));
  Serial.println(blockCounter);
#endif

  if (size > maxsize) {
#ifdef DEBUG_PRINT_ERROR
    Serial.println(F("ERROR: invalid maxsize - 1"));
#endif
    return false;
  }

  unsigned long timeout = millis() + 1000;
  while ((recvcount == 0) || (recvcount != size)) {
    while (obd->available()) {
      data = obdRead();
      s[recvcount] = data;
      recvcount++;
      if ((size == 0) && (recvcount == 1)) {
        size = data + 1;
        if (size > maxsize) {
#ifdef DEBUG_PRINT_ERROR
          Serial.print(F("ERROR: invalid maxsize - 2 size="));
          Serial.print(size);
          Serial.print(F(", maxsize="));
          Serial.println(maxsize);
#endif
          return false;
        }
      }
      if ((ackeachbyte) && (recvcount == 2)) {
        if (data != blockCounter) {
#ifdef DEBUG_PRINT_ERROR
          Serial.println(F("ERROR: invalid blockCounter"));
#endif
          disconnect();
          errorData++;
          return false;
        }
      }
      if ( ((!ackeachbyte) && (recvcount == size)) ||  ((ackeachbyte) && (recvcount < size)) ) {
        if (init_delay) {
          delay(30);
        } else {
          delay(3);
        }

        obdWrite(data ^ 0xFF);  // send complement ack
      }
      timeout = millis() + 1000;
    }
    if (millis() >= timeout) {
      Serial.println(F("ERROR: timeout"));
      disconnect();
      errorTimeout++;
      return false;
    }
  }
  // show data
#ifdef DEBUG_DEEP
  Serial.print(F("IN: sz="));
  Serial.print(size);
  Serial.print(F(" data="));
#endif
  for (int i = 0; i < size; i++) {
    uint8_t data = s[i];
#ifdef DEBUG_DEEP
    Serial.print(data, HEX);
    Serial.print(F(" "));
#endif
  }
#ifdef DEBUG_DEEP
  Serial.println();
#endif
  blockCounter++;
  return true;
}

bool VWOBD2::KWPSendAckBlock() {
#ifdef DEBUG_DEEP
  Serial.print(F("---KWPSendAckBlock blockCounter="));
  Serial.println(blockCounter);
#endif
  char buf[32];
  sprintf(buf, "\x03%c\x09\x03", blockCounter);
  return (KWPSendBlock(buf, 4));
}





/*

   readConnectBlocks

*/
static char s[128];
bool VWOBD2::readConnectBlocks() {
  // read connect blocks
#ifdef DEBUG_DEEP
  Serial.println(F("------readconnectblocks"));

  String info;
#endif

  while (true) {
    int size = 0;
    if (!(KWPReceiveBlock(s, 128, size))) {
#ifdef DEBUG_PRINT_ERROR
      Serial.println(F("ERROR:readConnectBlocks: KWPReceiveBlocks failed"));
#endif
      return false;
    }
    if (size == 0) {
#ifdef DEBUG_PRINT_ERROR
      Serial.println(F("ERROR:readConnectBlocks: empty answer"));
#endif
      return false;
    }
    if (s[2] == '\x09') {
      //      Serial.println(F("readConnectBlocks: got 09 - end of message?"));
      break;
    }
    if (s[2] != '\xF6') {
#ifdef DEBUG_PRINT_ERROR
      Serial.println(F("ERROR:readConnectBlocks: unexpected answer"));
#endif
      disconnect();
      errorData++;
      return false;
    }
#ifdef DEBUG_DEEP
    String text = String(s);
    info += text.substring(3, size - 2);
#endif
    if (!KWPSendAckBlock()) return false;
  }
#ifdef DEBUG_DEEP
  Serial.print(F("label="));
  Serial.println(info);
#endif

  return true;
}

