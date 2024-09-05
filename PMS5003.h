#ifndef PMS5003_H
#define PMS5003_H

#include <SoftwareSerial.h>

class PMS5003 {
public:
    PMS5003(uint8_t rxPin, uint8_t txPin);
    void begin();
    bool read(int &pm10, int &pm25, int &pm100);

private:
    SoftwareSerial _serial;
    static const uint8_t _dataLen = 32;
    uint8_t _buf[_dataLen];
    bool _findHeader();
};

#endif
