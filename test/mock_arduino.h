/*
 * This file is for including from the library src and has
 * defns for the Arduino interfaces and objects used by
 * the library. This allows me to compile and test on
 * whatever system I'm using to develop rather than on a
 * physical Arduino itself - which is quite painful and slow.
 */
#pragma once

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define lowByte(w) ((unsigned char) ((w) & 0xff))
#define highByte(w) ((unsigned char) ((w) >> 8))
#define _CONST const

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

typedef unsigned char byte;
typedef unsigned short int word;

class Stream {
public:
    virtual int write(const unsigned char*, int) = 0;
    virtual int write(const unsigned char) = 0;
    virtual char peek() = 0;
    virtual const char read() = 0;
    virtual int available() = 0;
};

class Print {
public:
    virtual int print(const char*) = 0;
    virtual int println(const char*) = 0;
    virtual int println(const char) = 0;
    virtual int print(int) = 0;
    virtual int print(int, int) = 0;
    virtual int println() = 0;
};


unsigned long millis();
void delay(unsigned long ms);
unsigned long millis();

// WMath.cpp
long map(long, long, long, long, long);

void initialize_mock_arduino();

#include "fake_serial.h"
