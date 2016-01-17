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

#define COMMAND_CHAR_SIZE 5
#define SERIAL_DELAY_MS   10
#define WAIT_MS           500
#define CC_SEPARATOR      ","
#define CC_COMMAND        '_'
#define CC_STRING         's'

// Command definitions
#define CCMSG_STOP          "STOP"
#define CCMSG_SPEED         "SPEE"
#define CCMSG_SEQ           "SEQQ"
#define CCMSG_TOP           "TOPP"
#define CCMSG_PAUSE         "PAUS"
#define CCMSG_BEEP          "BEEP"
#define CCMSG_PROG          "PROG"
#define CCMSG_RUN           "RUNN"
#define CCMSG_CLEAR         "CLER"
#define CCMSG_BCRAWL        "BCRL"
#define CCMSG_BIG           "BIGG"
#define CCMSG_NORMAL        "NORM"
#define CCMSG_BOLD          "BOLD"
#define CCMSG_ITALIC        "ITLC"
#define CCMSG_FLASH         "FLSH"
#define CCMSG_FORECOLOR     "FORE"
#define CCMSG_BACKCOLOR     "BACK"
#define CCMSG_SETTIME       "SETT"
#define CCMSG_CRAWL         "CRWL"
#define CCMSG_JUMP          "JUMP"
#define CCMSG_WIPEUP        "WIPU"
#define CCMSG_WIPEDN        "WIPD"
#define CCMSG_CAPS          "CAPS"
#define CCMSG_SHIFT         "SHFT"
#define CCMSG_GRAPH         "GRPH"
#define CCMSG_MAGIC         "MAGC"
#define CCMSG_TIME          "TIME"

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

int _strncasecmp(const char*, const char*, size_t);

class CC784Arduino {
public:
    CC784Arduino(Print &p): _logger(&p), _rx_timeout_ms(WAIT_MS) {};

    void begin(Stream &serial) {
        _serial = &serial;
    }

    int sendCommand(const char*);
    int sendString(const char*);
    int sendString(const char*, unsigned long);
    int processColorCellsProtocol(const char*);

    CCMetrics* getMetrics() {
        return &_metrics;
    }

protected:

    int validateChar(const char c);
    int sendControlCode(unsigned int);
    Stream* _serial;
    Print* _logger;
    CCMetrics _metrics;
    unsigned long _rx_timeout_ms;
};
#endif

