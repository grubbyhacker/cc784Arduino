/*
 * These tests are designed to run in the host developement environment,
 * not on an Arduino. This is so that I can quickly change and test the
 * logic rather than the long upload/serial debugging otherwise required.
 */
#include <assert.h>
#include <iostream>

#include "mock_arduino.h"
#include "CC784Arduino.h"

using namespace std;

class FakeStream: public Stream {
public:
    int write(const unsigned char* m, int) {
        return 1;
    }
    int write(const unsigned char c) {
        _b = c;
        return 1;
    }
    char peek() {
        return -1;
    }
    const char read() {
        return _b;
    }
    int available() {
        return 1;
    }
    char _b;
};

class FakePrint: public Print {
public:
    int print(const char* m) {
        cout << m;
        return 1;
    }
    int println() {
        cout << endl;
        return 1;
    }
    int println(const char m) {
        cout << m << endl;
        return 1;
    }
    int println(const char* m) {
        cout << m << endl;
        return 1;
    }
    int print(int i) {
        cout << i;
        return 1;
    }
    int print(int i, int f) {
        cout <<  i;
        return 1;
    }
};

typedef struct {
    const char* tcname; // Short description of test case
    const char* msg;    // Protocol test
    int ret;            // Expected return value
} TestCase;

TestCase tc[] = {
    {"Empty String.", "", 1},
    {"Single command", "_STOP", 1},
    {"Single string", "This is a string", 1},
    {"Command then string", "_STOPHello", 1},
    {"String then command", "Hello_STOP", 1},
    {"Multiple commands", "_STOP_PROG_SEQQ_RUNN", 1},
    {"command with too few chars", "_STO", 0},
    {"command with too few chars after string", "abc_STO", 0},
    {"Underscore hell", "abc______________", 0},
    {"Markup.", "This is an example of _BOLDtext_NORM.", 1},
    {"Illegal string chars.", "This is legal but \x01is not.", 0},
    {"Illegal chars mixed with commands.", "_STOP\x02", 0},
    {"illegal chars at front.", "\x03_SEQQ", 0},
    {"illegal chars boundary 1", "^", 0},
    {"illegal chars boundary 1", "\x5f", 0},
    {"illegal chars boundary 1", "\x7e", 0},
    {"illegal chars boundary 1", "\x1f", 0}
};

void testProtocol(const char* tcname, const char* msg, int ret) {
    FakePrint p;
    FakeStream s;
    CC784Arduino cc(p);
    cc.begin(s);
    cout << endl << endl << "Testing: " << tcname << ": " << msg <<  endl;
    assert(ret == cc.processColorCellsProtocol((msg)));
}

int main(int argc, char **argv) {
    for (int i=0; i<sizeof(tc)/sizeof(TestCase); i++) {
        testProtocol(tc[i].tcname, tc[i].msg, tc[i].ret);
    }
}

