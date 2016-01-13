/*
 *  Implements a simple interface to the Colorcells CC784 LED sign using TTL serial.
 *  
 *  The CC784 runs at 5v but was built for RS232. So it uses inverse logic (1 is
 *  ~0v and 0 is ~5v.) Its full specification is:
 *  
 * SERIAL_8N2 :=
 *  data: 8
 *  parity: none
 *  stop: 2
 *
 *  In practice I have not found it necessary to enable 2 stop bits so I stopped
 *  setting it up. But Arduino HardwareSerial does not support inverse logic, only
 *  SoftwareSerial does.
 *  
 *  I tested SoftwareSerial using an Arduino UNO at 300 baud and the inverse
 *  logic bit set, and it works fine. Unfortunately I could not get a reliable
 *  version of SoftwareSerial to work at 300 baud on the ESP8266. So I settled
 *  for HardwareSerial plus a cd4049 inverter IC.
 *  
 *  The protocol will be documented on github.
 */
#ifndef CC784Arduino_h
#define CC784Arduino_h

#define COMMAND_CHAR_SIZE 15
#define SERIAL_DELAY_MS   10
#define WAIT_MS           500
#define ERROR_TIMEOUT     -1
#define ERROR_BADCOMMAND  -2
#define CC_SEPARATOR      ","
#define CC_COMMAND        'c'
#define CC_STRING         's'

// Command definitions
#define CCMSG_STOP          "STOP"
#define CCMSG_SPEED         "SPEED"
#define CCMSG_SEQ           "SEQ"
#define CCMSG_TOP           "TOP"
#define CCMSG_PAUSE         "PAUSE"
#define CCMSG_BEEP          "BEEP"
#define CCMSG_PROG          "PROG"
#define CCMSG_RUN           "RUN"
#define CCMSG_CLEAR         "CLEAR"
#define CCMSG_BCRAWL        "BCRAWL"
#define CCMSG_BIG           "BIG"
#define CCMSG_NORMAL        "NORMAL"
#define CCMSG_BOLD          "BOLD"
#define CCMSG_ITALIC        "ITALIC"
#define CCMSG_FLASH         "FLASH"
#define CCMSG_FORECOLOR     "FORECOLOR"
#define CCMSG_BACKCOLOR     "BACKCOLOR"
#define CCMSG_SETTIME       "SETTIME"
#define CCMSG_CRAWL         "CRAWL"
#define CCMSG_JUMP          "JUMP"
#define CCMSG_WIPEUP        "WIPEUP"
#define CCMSG_WIPEDN        "WIPEDN"
#define CCMSG_CAPS          "CAPS"
#define CCMSG_SHIFT         "SHIFT"
#define CCMSG_GRAPH         "GRAPH"
#define CCMSG_MAGIC         "MAGIC"
#define CCMSG_SETADDR       "SETADDR"
#define CCMSG_TIME          "TIME"
#define CCMSG_STOPADDR      "STOPADDR"

typedef struct {
  char cmd[COMMAND_CHAR_SIZE];
  byte ascii;
} CommandT;

typedef struct {
  int txBytes;
  int rxBytes;
  int countTimeouts;
  int countIllegalCharsOnWire;
  int countAttemptedCommands;
  int countStrings;
  int countBadChars;
  int countBadCommands;
  int countProtocolErrors;
} CCMetrics;

int strncasecmp( _CONST char*, _CONST char*, size_t);

class CC784Arduino {
  public:
    CC784Arduino(Print* p): _logger(p), _rx_timeout_ms(WAIT_MS) {};

    void begin(Stream &serial) {
      _serial = &serial;
    }

    int sendCommand(const char*);
    int sendString(const char*);
    int sendString(const char*, unsigned long);
    int processColorCellsProtocol(char*);

    CCMetrics* getMetrics() { return &_metrics;}

  protected:
    int sendControlCode(unsigned int);

    Stream* _serial;
    Print* _logger;
    CCMetrics _metrics;
    unsigned long _rx_timeout_ms;
};
#endif

