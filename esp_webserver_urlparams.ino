#include <PString.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

#include "sessionlogger.h"
#include "colorcells784.h"
#include "html.h"

/*
 * TODOS: (x done, 0 dicarded, - undone)
 *
 * x add bail out logic that fails message if send rate is low (i.e. not in program mode.)
 * 0 2x crlf in all responses (to make it easier for curl users)
 * - pick a better delimiter so that comma can be used: try _, since its not allowed in sign anyway
 * x in cc7 guard against ascii values > 126 or 127 if i support delete
 * x think about just having two methods: command & string -> implemented in addition to the others
 * x collect statistics and serve them on root ("/")
 * 0 keep track of state and prevent any command from being processed if sign isn't stopped.
 * x handle sign communication errors, report back to caller
 * 0 consider a more robust protocol
 * - support graphics and colors natively
 * x fail call to sequence for >32
 * x test limit of sign and fail if attempt goes over limit
 * x make sequence always issue a stop, seq then run
 * x split colorcells logic and protocol stuff to its own class
 * x rename all usages of "control" with "logger"
 * x make /program issue "PROG" automaticaly (support "/program/3")
 * x case insensitive for C: and S:
 * x change led pin to use #define
 * x make all methods light LED during processing
 * x change all references to commands to use their #define names
 * x add a form on root "/" to submit each bank, and handl form submissions
 * x change pstring to be a circular buffer? (that's hard...) -> made it reset at the end
 * - modify Esp8266WebServer to allow streaming of response
 * - add feature to store results of program into eprom and api to retrieve them
 * - add feature to upload a file of 1-liners to the sign (and api)
 * - add set time feature
 *
 * Special thanks to: http://www.textfixer.com/html/compress-html-compression.php for compressing my awful html
 * Special thanks to: http://www.html.am/ for providing great templates that I destroyed with my awful html
 * Special thanks to: http://www.dirtymarkup.com/ for pretty printing my awful html
 * Special thanks to: The makers of the PString class at Arduiniana.org
 * Special thanks to: http://todbot.com/blog/2008/06/19/how-to-do-big-strings-in-arduino/
 */
#define RXPIN             13
#define TXPIN             14
#define RELAYPIN          4
#define LEDPIN            0

#define SESSION_BUFFER_SIZE    512
#define HTML_BUFFER_SIZE       8192

const char* ssid = "**********";
const char* password = "****************";

char sessionbuffer[SESSION_BUFFER_SIZE];
char htmlbuffer[HTML_BUFFER_SIZE];
unsigned long lastreboot = 0;

ESP8266WebServer server(80);
SoftwareSerial ss(RXPIN, TXPIN);
SessionLogger logger(ss, sessionbuffer, SESSION_BUFFER_SIZE);
ColorCells784 cc(&logger);

int metrics_count_requests = 0;
int metrics_200_responses = 0;
int metrics_300_responses = 0;
int metrics_400_responses = 0;
int metrics_500_responses = 0;

/*
 * TODO: find citation from where I lifted this implementation off of the web.
 */
void urldecode(char *dst, const char *src)
{
  char a, b;
  while (*src) {
    if ((*src == '%') &&
        ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

void beginHandler(char *msg = 0) {
  logger.resetBuffer();
  metrics_count_requests++;
  digitalWrite(LEDPIN, 1); // Turns LED off
  logger.resetBuffer();
  if (msg) {
    logger.println(msg);
  }
}

void endHandler(int code, const char* msg) {
  server.send(code, "text/plain", msg);
  switch  (code / 100) {
    case 2:
      metrics_200_responses++;
      break;
    case 3:
      metrics_300_responses++;
      break;
    case 4:
      metrics_400_responses++;
      break;
    case 5:
      metrics_500_responses++;
      break;
  }
  digitalWrite(LEDPIN, 0); // Turns LED back on
}

void endHandler(int code) {
  endHandler(code, sessionbuffer);
}

void handleRoot() {
  beginHandler();
  PString p(htmlbuffer, HTML_BUFFER_SIZE);
  p.begin();
  printProgStr(root_header, &p);

  p.print("<tr><td>HTTP Requests</td><td>");
  p.print(metrics_count_requests);
  p.println("</td></tr>");

  p.print("<tr><td>200 Responses</td><td>");
  p.print(metrics_200_responses);
  p.println("</td></tr>");

  p.print("<tr><td>300 Responses</td><td>");
  p.print(metrics_300_responses);
  p.println("</td></tr>");

  p.print("<tr><td>400 Responses</td><td>");
  p.print(metrics_400_responses);
  p.println("</td></tr>");

  p.print("<tr><td>500 Responses</td><td>");
  p.print(metrics_500_responses);
  p.println("</td></tr>");

  p.print("<tr><td>Serial bytes TX</td><td>");
  p.print(cc.getMetrics()->txBytes);
  p.println("</td></tr>");

  p.print("<tr><td>Serial bytes RX</td><td>");
  p.print(cc.getMetrics()->rxBytes);
  p.println("</td></tr>");

  p.print("<tr><td>Timeouts</td><td>");
  p.print(cc.getMetrics()->countTimeouts);
  p.println("</td></tr>");

  p.print("<tr><td>RX violations</td><td>");
  p.print(cc.getMetrics()->countIllegalCharsOnWire);
  p.println("</td></tr>");

  p.print("<tr><td>Attempted Commands</td><td>");
  p.print(cc.getMetrics()->countAttemptedCommands);
  p.println("</td></tr>");

  p.print("<tr><td>Bad Commands</td><td>");
  p.print(cc.getMetrics()->countBadCommands);
  p.println("</td></tr>");

  p.print("<tr><td>Attempted Strings</td><td>");
  p.print(cc.getMetrics()->countStrings);
  p.println("</td></tr>");

  p.print("<tr><td>Illegal Chars</td><td>");
  p.print(cc.getMetrics()->countBadChars);
  p.println("</td></tr>");

  p.print("<tr><td>Protocol parse errors</td><td>");
  p.print(cc.getMetrics()->countProtocolErrors);
  p.println("</td></tr>");

  printProgStr(root_footer, &p);
  server.send(200, "text/html", htmlbuffer);
}

void handleNotFound() {
  beginHandler();
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  endHandler(404, message.c_str());
}

void handleStop() {
  beginHandler();
  if (cc.sendCommand(CCMSG_STOP)) {
    endHandler(200);
  } else {
    endHandler(500);
  }
}

void handleRun() {
  beginHandler();
  if (cc.sendCommand(CCMSG_RUN)) {
    endHandler(200);
  } else {
    endHandler(500);
  }
}

/*
 * Allows caller to specifiy the sequence of messages. Any order is allowed and up to
 * 32 messages (0-9) are allowed. Syntax of the command is: /sequence?value=\d+
 *
 * Automatically calls STOP before sequencing and RUN afterwards.
 */
void handleSequence() {
  beginHandler();
  char paramValue[33];
  memset(paramValue, 0, 33);

  if (server.args() == 0) {
    logger.println("Bad Request: provide a sequence of digits like this: /sequence?value=1234.");
    endHandler(400);
    return;
  }

  server.arg(0).toCharArray(paramValue, 33);
  logger.print("Sequence: ");
  logger.println(paramValue);

  if (strlen(paramValue) > 32) {
    logger.println("Bad Request: Maximum of 32 items in sequence.");
    endHandler(400);
    return;
  }

  char *pch = paramValue;
  while (*pch) {
    logger.print("Examining sequence character: ");
    logger.print(*pch);
    logger.print(" ");
    logger.print(*pch, DEC);
    logger.println();
    if ((*pch) < 48 || (*pch) > 57) {
      logger.println("Bad Request: Only numbers can be passed to sequence.");
      endHandler(400);
      return;
    }
    ++pch;
  }

  if(cc.sendCommand(CCMSG_STOP) &&
    cc.sendCommand(CCMSG_SEQ) &&
    cc.sendString(paramValue) &&
    cc.sendCommand(CCMSG_RUN))
  {
    endHandler(200);
    return;
  }
  endHandler(500);
}

void handleRebootDisplay() {
  beginHandler();
  if ((0 == lastreboot) || ((millis() - lastreboot) > 1000)) {
    digitalWrite(RELAYPIN, 1);
    delay(5000);
    digitalWrite(RELAYPIN, 0);
    lastreboot = millis();
    endHandler(200, "Reboot Completed.");
  } else {
    endHandler(503, "Too soon.");
  }
}

void programBank(const char* bank) {
  beginHandler();
  if (!server.hasArg("msg")) {
    endHandler(400, "Missing msg parameter.");
    return;
  }
  programBank(bank, server.arg("msg").c_str());
}

void programBank(const char* bank, const char* msg) {
  char decodedParam[255];
  logger.resetBuffer();

  if (strlen(bank)>1 || !isdigit(bank[0])) {
    endHandler(400, "The bank parameter must be a single digit (0-9).");
    return;
  }

  urldecode(decodedParam, msg);
  ss.print("Decoded arg: ");
  ss.println(decodedParam);

  if (cc.sendCommand(CCMSG_STOP) &&
      cc.sendCommand(CCMSG_PROG) &&
      cc.sendString(bank) &&
      cc.sendCommand(CCMSG_CLEAR) &&
      cc.processColorCellsProtocol(decodedParam) &&
      cc.sendCommand(CCMSG_RUN))
  {
        endHandler(200);
        return;
  }
  endHandler(400);
}

void setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(LEDPIN, 0);
  digitalWrite(RELAYPIN, 0);

  cc.setup();
  ss.begin(9600);
  WiFi.begin(ssid, password);
  ss.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ss.print(".");
  }
  ss.println("");
  ss.print("Connected to ");
  ss.println(ssid);
  ss.print("IP address: ");
  ss.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/stop", handleStop);
  server.on("/run", handleRun);
  server.on("/sequence", handleSequence);
  server.on("/rebootdisplay", handleRebootDisplay);

  server.on("/cmd", []() {
    beginHandler();
    if (cc.sendCommand(server.arg(0).c_str())) {
      endHandler(200);
    } else {
      endHandler(500);
    }
  });

  server.on("/str", []() {
    beginHandler();
    if (cc.sendString(server.arg(0).c_str())) {
      endHandler(200);
    } else {
      endHandler(500);
    }
  });

  // emulate rest server dispatching multiple URIs
  server.on("/program/0", []() {
    programBank("0");
  });
  server.on("/program/1", []() {
    programBank("1");
  });
  server.on("/program/2", []() {
    programBank("2");
  });
  server.on("/program/3", []() {
    programBank("3");
  });
  server.on("/program/4", []() {
    programBank("4");
  });
  server.on("/program/5", []() {
    programBank("5");
  });
  server.on("/program/6", []() {
    programBank("6");
  });
  server.on("/program/7", []() {
    programBank("7");
  });
  server.on("/program/8", []() {
    programBank("8");
  });
  server.on("/program/9", []() {
    programBank("9");
  });

  // This is for the form submission: ?bank=1&msg=s:foo
  server.on("/program", []() {
    beginHandler();
    if (!server.hasArg("bank")) {
      endHandler(400, "Missing bank parameter.");
      return;
    }
    if (!server.hasArg("msg")) {
      endHandler(400, "Missing msg parameter.");
      return;
    }
    programBank(server.arg("bank").c_str(), server.arg("msg").c_str());
  });

  server.onNotFound(handleNotFound);
  server.begin();
  ss.println("HTTP server started");
  ss.println(" *  READY  *");
  ss.println(">");
}

void loop() {
  server.handleClient();
  delay(100); // included to make sure esp8266 maintains network
}
