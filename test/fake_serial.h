#pragma once

#include <iostream>

class FakeSerial {
	public:
		  void begin(unsigned long);
		    void end();
		      size_t write(const unsigned char*, size_t);
};

extern FakeSerial Serial;
