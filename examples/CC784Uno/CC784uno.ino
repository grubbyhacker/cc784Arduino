/*
 * Sample to show how to connect and UNO to the CC784 with a very simple
 * command interpreter.
 * 
 * NOTE: the UNO is a 5v device and so is the sign, and they can talk to
 * each other just fine using SoftwareSerial with the inverted bit on.
 * 
 * Pin guide:
 * UNO_PIN  CC784_PIN
 * RX       3         // RX = the RX pin you use for SoftwareSerial
 * TX       6         // TX = the TX pin you use for SoftwareSerial
 * GROUND   9
 * GROUND   2
 */
#include <CC784Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial ccSerial(13,12,1);
CC784Arduino cc(Serial);

typedef void (*shellHandler)(char*);

typedef struct {
  char* command;
  char* description;
  shellHandler handler;
} shellCommandT;

shellCommandT shellCommands[] = {
  {"help", "Displays all commands", &helpHandler},
  {"stop", "Stops the display.", &stopHandler},
  {"run", "Starts the display.", &runHandler},
  {"seq", "Sets the sequence: [0-9]", &seqHandler},
  {"prog", "Changes the value of a message bank: [0-9] protocolmessage", &progHandler}
};

void helpHandler(char*) {
  for (int i=0; i<sizeof(shellCommands)/sizeof(shellCommandT); i++) {
    Serial.print(shellCommands[i].command);
    Serial.print("\t");
    Serial.println(shellCommands[i].description);
  }
}

void stopHandler(char*) {
  cc.stop();
}

void runHandler(char*) {
  cc.run();
}

void seqHandler(char* msg) {
  char* pch = strtok(msg, " \t");
  if (pch == NULL) {
    Serial.println("Sequence sytax error");
    return;
  }
  pch = strtok (NULL, " \t");
  if (pch == NULL) {
    Serial.println("Sequence sytax error");
    return;
  }
  cc.setSequence(pch);
}

void progHandler(char* msg) {
  char* pch = strtok(msg, " \t");
  if (pch == NULL) {
    Serial.println("Program sytax error");
    return;
  }
  pch = strtok (NULL, " \t");
  if (pch == NULL) {
    Serial.println("Program sytax error");
    return;
  }
  char *bank = pch;
  pch = pch + strlen(bank) + 1;
  Serial.println(bank);
  Serial.println(pch);
  cc.programBank(bank[0], pch);  
}

void prompt() {
  Serial.println(" *  READY  *");
  Serial.println(">");
}

void setup() {
  Serial.begin(115200);
  ccSerial.begin(300);
  cc.begin(ccSerial);
  prompt();
}

void loop() {
  int incomingByte;
  char buf[255];
  memset(buf,0,sizeof(buf));

  // Read one line of data from Serial and dispatch it
  if (Serial.available()) {
    int idx = 0;
    while (1) {
      incomingByte = Serial.read();
      if (incomingByte == '\n') break;
      if (idx >= 255) break;
      if (incomingByte == -1) {
        delay(100);
        continue;
      }
      buf[idx++] = incomingByte;
    }

    if (strlen(buf) < 3) {
      Serial.println("Syntax error");
    }

    for (int i=0; i<sizeof(shellCommands)/sizeof(shellCommandT); i++) {
      if (0 == strncasecmp(buf, shellCommands[i].command, 3)) {
        shellCommands[i].handler(buf);
        prompt();
        return; 
      }
    }

    Serial.println("Invalid command.");
    prompt();
  }
  delay(250);
}

