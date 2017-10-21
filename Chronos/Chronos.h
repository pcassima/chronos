#ifndef Chronos_H
#define Chronos_H

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class Chronos {
public:
  //Public functions
  Chronos();
  ~Chronos();

  //Public variables

private:
  //Private functions


  //Private variables
  int SER;    //INPUT
  int OE;    //blank display on high
  int RCLK;  //Latch clock
  int SRCLK; //Shift clock
  int SRCLR; //RESET on low
  int digit0; //digit select Bit 0 > digits
  int digit1; //digit select Bit 1 > digits
  int digit2; //digit select bit 2 > dots

protected:
  //Protected functions


  //protected variables
    
};

#endif
