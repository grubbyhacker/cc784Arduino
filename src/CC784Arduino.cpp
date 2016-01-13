#include <arduino.h>
#include "CC784Arduino.h"

/*
 * TODO: find reference for where I lifted this implementation off the web if necessary.
 */
int strncasecmp(
  _CONST char *s1,
  _CONST char *s2,
  size_t n)
{
  if (n == 0)
    return 0;

  while (n-- != 0 && tolower(*s1) == tolower(*s2))
  {
    if (n == 0 || *s1 == '\0' || *s2 == '\0')
      break;
    s1++;
    s2++;
  }

  return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}

CommandT _commands[] = {
  {CCMSG_STOP,      1},
  {CCMSG_SPEED,     2},
  {CCMSG_SEQ,       4},
  {CCMSG_TOP,       5},
  {CCMSG_PAUSE,     6},
  {CCMSG_BEEP,      7},
  {CCMSG_PROG,      9},
  {CCMSG_RUN,       13},
  {CCMSG_CLEAR,     14},
  {CCMSG_BCRAWL,    15}, // Bcrawl
  {CCMSG_BIG,       16},
  {CCMSG_NORMAL,    17},
  {CCMSG_BOLD,      18},
  {CCMSG_ITALIC,    19},
  {CCMSG_FLASH,     20},
  {CCMSG_FORECOLOR, 21}, // Fore Color
  {CCMSG_BACKCOLOR, 22}, // Back Color
  {CCMSG_SETTIME,   23}, // Set Time
  {CCMSG_CRAWL,     24},
  {CCMSG_JUMP,      25},
  {CCMSG_WIPEUP,    26},
  {CCMSG_WIPEDN,    27},
  {CCMSG_CAPS,      28},
  {CCMSG_SHIFT,     29},
  {CCMSG_GRAPH,     30},
  {CCMSG_MAGIC,     31},
  {CCMSG_SETADDR,   94}, // Set Addr
  {CCMSG_TIME,      95}, // Time
  {CCMSG_STOPADDR,  126} // Stop Addr
};

/*
 * Sends a string of ASCII characters. Returns failure on any chars <32 (space) or >125, or
 * the two special codes 94 & 95 (used for Set Addr and Time.
 */
int CC784Arduino::sendString(const char *msg) {
  int errs = 0;
  _metrics.countStrings++;
  _logger->print("String: ");
  _logger->println(msg);
  while (*msg) {
    if (*msg > 125 || *msg < 32 || 94==*msg || 95==*msg) {
      _logger->print("Bad character found in string: ");
      _logger->print(*msg, DEC);
      _logger->println();
      _metrics.countBadChars++;
      return 0;
    }
    if (0 == sendControlCode(*msg)) {
      if (++errs > 5) {
        _logger->println("Too many serial RX errors, bailing.");
        return 0;
      }
      
    }
    msg++;
  }
  return 1;
}

/*
 * This version temporarily sets the timeout to a user defined value. This
 * is needed only for the SETTIME command which can take 2-5 seconds before
 * it responds to the last digit.
 */
int CC784Arduino::sendString(const char *msg, unsigned long rx_timeout_ms) {
  _rx_timeout_ms = rx_timeout_ms;
  sendString(msg);
  _rx_timeout_ms = WAIT_MS;
}

int CC784Arduino::processColorCellsProtocol(char *input) {
  char* pch = strtok (input, CC_SEPARATOR);
  while (pch != NULL)
  {
    _logger->print("Token: ");
    _logger->println(pch);

    switch (tolower(*pch)) {
      case CC_COMMAND:
        sendCommand(&pch[2]);
        break;
      case CC_STRING:
        sendString(&pch[2]);
        break;
      default:
        _logger->print("Bad command qualifier, must be one of [CcSs]: ");
        _logger->println(pch);
        _metrics.countProtocolErrors++;
        return 0;
        break;
    }
    pch = strtok (NULL, CC_SEPARATOR);
  }
  return 1;
}

int CC784Arduino::sendCommand(const char *cmd) {
  _logger->print("Command: ");
  _logger->println(cmd);
  for (int i = 0; i < sizeof(_commands) / sizeof(CommandT); i++) {
    if (strncasecmp(cmd, _commands[i].cmd, COMMAND_CHAR_SIZE) == 0) {
      _metrics.countAttemptedCommands++;
      return sendControlCode(_commands[i].ascii);
   }
  }
  _metrics.countBadCommands++;
  _logger->println("Bad Command.");
  return 0;
}

int CC784Arduino::sendControlCode(unsigned int ch) {
  _metrics.txBytes++;
  while (-1 != _serial->peek()) {
    _logger->print("found: ");
    _logger->println(_serial->read());
    _metrics.rxBytes++;
    _metrics.countIllegalCharsOnWire++;
    delay(1);
  }
  _logger->print("Sending: ");
  _logger->print(ch, DEC);
  _serial->write(ch);
  _metrics.txBytes++;
  _logger->print(":");
  unsigned int retry = 1;
  unsigned long startmillis = millis();
  while (!_serial->available()) {
    if ((millis() - startmillis) > _rx_timeout_ms) {
      _logger->println("Timeout.");
      _metrics.countTimeouts++;
      return 0;
    }
    if ((retry++ % 10) == 0) {
      _logger->print(".");
    }
    delay(SERIAL_DELAY_MS);
  }
  _logger->println(_serial->read());
  _metrics.rxBytes++;
  return 1;
}
