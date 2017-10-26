#ifndef Chronos_H
#define Chronos_H

#include <Arduino.h>

class Chronos {
    public:
        //public functions:
        Chronos();
        ~Chronos();
        void writeInt(int number);
        void setBrightness(int newBrightness);
        
    protected:
        //protected functions:
        
    private:
        //private functions
        void shiftOutShape(byte shape);
        void chooseDigit(byte digit);
        void writeDigit(byte shape, byte digit);
        
};
#endif
