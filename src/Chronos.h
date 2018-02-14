#ifndef Chronos_H
#define Chronos_H

#include <Arduino.h>
#include <Wire.h>

class DateTime{
    public:
        DateTime (uint32_t t = 0);
        DateTime (  uint16_t year,
                    uint8_t month,
                    uint8_t day,
                    uint8_t hour = 0,
                    uint8_t min = 0,
                    uint8_t sec = 0);
        DateTime (const char* date, const char* time);
        
        uint16_t year() const           { return 2000 + yOff; }
        uint8_t month() const           { return m; }
        uint8_t day() const             { return d; }
        uint8_t hour() const            { return hh; }
        uint8_t minute() const          { return mm; }
        uint8_t second() const          { return ss; }
        uint8_t dayOfTheWeek() const;

        long secondstime() const;

        uint32_t unixtime(void) const;

    protected:
        uint8_t yOff, m, d, hh, mm, ss;
        
};

class Chronos {
    public:
        //public functions:
        Chronos(bool debug);
        ~Chronos();
        void writeInt(int number);
        void setBrightness(uint8_t newBrightness);
        void showTemp(float temperature);
        void showTemp();
        void showTime(int hours, int minutes);
        void setIntRollOver(bool rollOver);

        float getTemperature();

        
    protected:
        //protected functions:
        
    private:
        //private functions
        void blankDisp();
        void shiftOutShape(uint8_t shape);
        void chooseDigit(uint8_t digit);        
        void writeDigit(uint8_t shape, uint8_t digit);

        uint8_t registerDecode(uint8_t val);
        uint8_t registerEncode(uint8_t val);
};
#endif
