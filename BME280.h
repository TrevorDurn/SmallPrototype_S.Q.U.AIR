#ifndef BME280_H
#define BME280_H

#include <Wire.h>

class BME280 {
public:
    bool begin(uint8_t addr = 0x76);
    void readCalibrationData();
    void read(float &temperature, float &humidity);

private:
    uint8_t _i2caddr;
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_H1, dig_H3;
    int16_t dig_H2, dig_H4, dig_H5;
    int8_t dig_H6;
    int32_t t_fine;

    int32_t readRawTemperature();
    int32_t readRawHumidity();
};

#endif
