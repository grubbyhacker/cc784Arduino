/*
 * SessionLogger wraps a SoftwareSerial interface and a PString buffer, essentially  
 * allowing me to output to both with a single command. It can be used just like any 
 * other object that implements Print (Serial, etc...) In this way I can use a
 * SoftwareSerial interface to do debug logging, and send the same debug log back
 * in the HTTP response body. I use both because I trim the response body to a 
 * fixed size buffer, and a bug might happen that prevents the response from being
 * sent, so the SoftwareSerial debug interface is useful then.
 */
#include <SoftwareSerial.h>
#include <PString.h>

class SessionLogger: public Print {

  public:  

    SessionLogger(SoftwareSerial ser, char* buf, size_t size)
      : _pstring(buf, size), _ser(ser)
    {}
      
    virtual size_t write(uint8_t b) {
      if (0 == _pstring.write(b)) {
        _pstring.begin(); // reset buffer it it fills up during processing.
        _pstring.write(b);
      }
      _ser.write(b);
      return 1;
    }
    
    void resetBuffer() {
      _pstring.begin();
    }

    void beginSerial(uint8_t bps) {
      _ser.begin(bps);
    }

    PString getPString() {
      return _pstring;
    }

  protected:
    PString _pstring;
    SoftwareSerial _ser;
};


