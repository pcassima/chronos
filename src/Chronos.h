/*
Chronos.h
Header file for the Chronos library.
Written for the Chronos_V1 shield,
developed by Fenix Computers.
Written by Pieter-Jan Cassiman
Version: 2.0
*/

#ifndef _CHRONOS_h
#define _CHRONOS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>

class Chronos {
	public:
		/*
		Public functions
		these functions can be called from
		anywhere
		*/
		//constructor and deconstructor for the library
		Chronos();
		~Chronos();

		//functions related to the display
		void setBrightness(uint8_t newBrightness);
		void writeDigit(uint8_t shape, uint8_t digit);
		void writeInt(int number);
		void writeFl(float number, uint8_t DP = 1);
		void writeTemp(float temperature);
		void writeTime(uint8_t hours, uint8_t minutes);

		//functions related to the DS3231
		float getTemp();

		void writeTemp();
		void writeTime();

		uint8_t getHours();
		uint8_t getMinutes();
		uint8_t getSeconds();

		uint8_t getDay();
		uint8_t getDOW();
		uint8_t getMonth();
		uint8_t getYear();

		void setHours(uint8_t Hours);
		void setMinutes(uint8_t Minutes);
		void setSeconds(uint8_t Seconds);

		void setTime(uint8_t Hours, uint8_t Minutes, uint8_t Seconds);

		void setDay(uint8_t Day);
		void setDOW(uint8_t DOW);
		void setMonth(uint8_t Month);
		void setYear(uint8_t Year);

		void setDate(uint8_t Day, uint8_t Month, uint8_t Year);


	protected:
		/*
		Protected functions
		these functions can be called from
		this class and any subclasses
		*/

	private:
		/*
		Private functions
		these functions can be called from
		within the class only
		*/
		void writePosInt(int number);
		void writeNegInt(int number);
		void writePosFl(float number, uint8_t DP);
		void writeNegFl(float number, uint8_t DP);
		void writePosTemp(float temperature);
		void writeNegTemp(float temperature);
		uint8_t decToBcd(uint8_t val);
		uint8_t bcdToDec(uint8_t val);
};

#endif