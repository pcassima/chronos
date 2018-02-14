/*
 * Example showing the capabilities of the
 * writeInt and setBrightness functions
 * Written by Pieter-Jan Cassiman
 */

#include <Chronos.h>

int number = 2017; //the number that we want to display

Chronos chronos(false); //initialising the library


void setup() {
  // put your setup code here, to run once:
  //Setting the brightness of the library to the maximum
  //value
    chronos.setBrightness(255);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Writing the number on the display
    chronos.writeInt(number);
}
