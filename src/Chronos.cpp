/*
Chronos.cpp
Header file for the Chronos library.
Written for the Chronos_V1 shield,
developed by Fenix Computers.
Written by Pieter-Jan Cassiman
Version: 2.0
*/
#include "Chronos.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>

//********************************* Variables **********************************

//Address-map for the DS3231
//All these values are taken straight from the datasheet
//Main TWI address of the DS3231
#define DS3231_ADDRESS 0x68
//Internal addresses for time/date keeping
#define DS3231_SECONDS_REGISTER 0x00
#define DS3231_MINUTES_REGISTER 0x01
#define DS3231_HOURS_REGISTER   0x02
#define DS3231_DAY_REGISTER     0x03
#define DS3231_DATE_REGISTER    0x04
#define DS3231_MONTH_REGISTER   0x05
#define DS3231_YEAR_REGISTER    0x06
//Internal addresses for the alarms
//Alarm 1
#define DS3231_A1_SECONDS       0x07
#define DS3231_A1_MINUTES       0x08
#define DS3231_A1_HOURS         0x09
#define DS3231_A1_DATE          0x0A
//Alarm2
#define DS3231_A2_MINUTES       0x0B
#define DS3231_A2_HOURS         0x0C
#define DS3231_A2_DATE          0x0D
//Internal addresses for control registers
#define DS3231_CONTROL_REGISTER 0x0E
#define DS3231_STATUS_REGISTER  0x0F
#define DS3231_AGING_OFFSET     0x10
//Internal addresses for temperature registers
#define DS3231_MSB_TEMP         0x11
#define DS3231_LSB_TEMP         0x12

//pins for the 595 shift register
const int SER = 9;    //INPUT
const int OE = 10;    //blank display on high
const int RCLK = 11;  //Latch clock
const int SRCLK = 12; //Shift clock
const int SRCLR = 13; //RESET on low

//pins for the multiplexer ship
const int ds0 = 8; //digit select Bit 0 > digits
const int ds1 = 7; //digit select Bit 1 > digits
const int ds2 = 6; //digit select bit 2 > dots

//initial brightness for the display (maximum)
uint8_t Brightness = 255;

/*
list with all the bytes needed for display numbers
onto the 7-segment display
the order for the bytes is as follows:
B7 B6 B5 B4 B3 B2 B1 B0
A  B  C  D  E  F  G  DP

the 7-segment display is arranged as follows:

        *---A---*
        |       |
        F       B
        |       |
        *---G---*
        |       |
        E       C
        |       |
        *---D---*
                    DP
 */

const uint8_t digits[] PROGMEM = {
									0b11111100,
									0b01100000,
									0b11011010,
									0b11110010,
									0b01100110,
									0b10110110,
									0b10111110,
									0b11100000,
									0b11111110,
									0b11110110,
									0b11000000,
									0b00000000
									};

//****************************** Public functions ******************************
Chronos::Chronos() {

    pinMode(SER, OUTPUT);
    pinMode(OE, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(SRCLK, OUTPUT);
    pinMode(SRCLR, OUTPUT);

    digitalWrite(OE, LOW);
    digitalWrite(SRCLR, HIGH);

    pinMode(ds0, OUTPUT);
    pinMode(ds1, OUTPUT);
    pinMode(ds2, OUTPUT);

}

Chronos::~Chronos() {
	/*
	Our library does not require a deconstructor
	*/
}

void Chronos::setBrightness(uint8_t newBrightness) {
    //this function will update the brightness
    //to a new value
    Brightness = newBrightness;
}

void Chronos::writeDigit(	uint8_t shape,
							uint8_t digit) {

    /*
     Function for writing data to a single digit
     We will write the data to the shift register
     Disable the output of the SR
     select the digit we want
     latch the data into the output register
     enable the output of the SR again
     we have our data displayed
     */

    //starting by shifting out the data for our shape
    for (uint8_t i = 0; i < 8; i++ ) {
        /*
         Shifting out the bits to the shift register
         in order to know which state the input needs
         to be, we shift over the bits to the least
         significant position and use and AND operation
         with 1 to see what state we need.
         */

        //setting the input to the required state
        digitalWrite(SER, (shape >> i) & 1);
        //pulsing the shift clock to store the
        //bit in the shift register
        digitalWrite(SRCLK, HIGH);
        digitalWrite(SRCLK, LOW);
    }

    //disabling the SR output
    digitalWrite(OE, HIGH);

    //In order to select the digit we need
    //we will do something similar to what we did
    //to select the bit to shift out.
    //Here the and function will be with 1,2,4
    //this is similar to a decimal to binary conversion
    digitalWrite(ds0, digit & 1);
    digitalWrite(ds1, digit & 2);
    digitalWrite(ds2, digit & 4);

    //pulsing the latch clock to bring the data on the output
    digitalWrite(RCLK, HIGH);
    digitalWrite(RCLK, LOW);

    //enabling the output again
    delayMicroseconds((255 - Brightness) << 1);
    digitalWrite(OE, LOW);

};

void Chronos::writeInt(int number) {
    /*
     This public accessible function will
     call the function to write a positive
     or negative integer to the display
     depending on the value of the integer
     */
    if (number < 0) {
		//Negative integer
        writeNegInt(number);
    }
    else {
		//Positive integer
        writePosInt(number);
    }
}

void Chronos::writeFl(	float number,
						uint8_t DP = 1) {
	/*
	This public accessible function will
	call the function to write a positive
	or negative float to the display
	depending on the value of the float
	*/
    if (number < 0) {
		//Negative float
        writeNegFl(number, DP);
    }
    else {
		//Positive float
        writePosFl(number, DP);
    }
}

void Chronos::writeTemp(float temperature) {
	/*
	This public accessible function will
	call the function to write a positive
	or negative temperature to the display
	depending on the value of the temperature
	*/
	if (temperature < 0) {
		//Negative temperature
		writeNegTemp(temperature);
	}
	else {
		//Positive temperature
		writePosTemp(temperature);
	}
}

void Chronos::writeTime(uint8_t hours,
						uint8_t minutes) {
	/*
    This function will write the time
    to the display
    */

    uint8_t digit3Value = minutes % 10;
    minutes -= digit3Value;
    uint8_t digit2Value = (minutes % 100) / 10;

    uint8_t digit1Value = hours % 10;
    hours -= digit1Value;
    uint8_t digit0Value = (hours % 100) / 10;

	writeDigit(0b11000000, 4);
    writeDigit(digits[digit3Value], 3);
    writeDigit(digits[digit2Value], 2);
    writeDigit(digits[digit1Value], 1);
    writeDigit(digits[digit0Value], 0);

}

float Chronos::getTemp() {
	/*
	This function will read both the
	temperature registers. After reading
	both bytes will be converted back to
	a floating point number.
	*/

	float temp3231;

	//starting the transmission the the DS3231
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(DS3231_MSB_TEMP);
	Wire.endTransmission();
	//We need to request to bytes from the DS3231
	Wire.requestFrom(DS3231_ADDRESS, 2);

	if (Wire.available()) {
		//if we receive "something" from the DS3231
		//we can read it into two bytes
		uint8_t tMSB = Wire.read();
		uint8_t tLSB = Wire.read();
		//we can now reconstruct the floating point number from
		//the two bytes.
		temp3231 = ((((short)tMSB << 8) | (short)tLSB) >> 6) / 4.0;
		//more information on how this works will follow later
	}

	else {
		/*
		If something goes wrong we give an obvious error value;
		the minimum value on the Celsius scale is -273.15
		so -999 is an obvious error value
		*/
		temp3231 = -999.0;
	}

	return temp3231;
}

uint8_t Chronos::getHours() {
	//Function needs to be written
	//taking into account the structure
	//of the register
}

uint8_t Chronos::getMinutes() {
	/*
	this function will read the minutes
	register and return the converted value
	The value is stored in binary coded decimal
	*/
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(DS3231_MINUTES_REGISTER);
	Wire.endTransmission();

	Wire.requestFrom(DS3231_ADDRESS, 1);
	return bcdToDec(Wire.read());
}

uint8_t Chronos::getSeconds() {
	/*
	this function will read the seconds
	register and return the converted value
	The value is stored in binary coded decimal
	*/
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(DS3231_SECONDS_REGISTER);
	Wire.endTransmission();

	Wire.requestFrom(DS3231_ADDRESS, 1);
	return bcdToDec(Wire.read());
}

uint8_t Chronos::getDay() {
	/*
	this function will read the day
	register and return the converted value
	The value is stored in binary coded decimal
	*/
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(DS3231_DAY_REGISTER);
	Wire.endTransmission();

	Wire.requestFrom(DS3231_ADDRESS, 1);
	return bcdToDec(Wire.read());
}

uint8_t Chronos::getDOW() {
	/*
	this function will read the Day of the Week
	register and return the converted value
	The value is stored in binary coded decimal
	*/
}

uint8_t Chronos::getMonth() {
	/*
	this function will read the months
	register and return the converted value
	The value is stored in binary coded decimal
	*/
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(DS3231_MONTH_REGISTER);
	Wire.endTransmission();

	Wire.requestFrom(DS3231_ADDRESS, 1);
	return bcdToDec(Wire.read());
}

uint8_t Chronos::getYear() {
	/*
	this function will read the year
	register and return the converted value
	The value is stored in binary coded decimal
	*/
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(DS3231_YEAR_REGISTER);
	Wire.endTransmission();

	Wire.requestFrom(DS3231_ADDRESS, 1);
	return bcdToDec(Wire.read());
}

void Chronos::setHours(uint8_t Hours) {
	/*
	Needs to be written taking into
	account the structure of the register
	*/
}

void Chronos::setMinutes(uint8_t Minutes) {
	/*
	This function will write the value
	to the minutes register
	first the value needs to be converted
	from a byte to a binary coded decimal
	*/

	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(DS3231_MINUTES_REGISTER);
	Wire.write(decToBcd(Minutes));
	Wire.endTransmission();
}

void Chronos::setSeconds(uint8_t Seconds) {
	/*
	This function will write the value
	to the seconds register
	first the value needs to be converted
	from a byte to a binary coded decimal
	*/

	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(DS3231_SECONDS_REGISTER);
	Wire.write(decToBcd(Seconds));
	Wire.endTransmission();
}

void Chronos::setDay(uint8_t Day) {

}

void Chronos::setDOW(uint8_t DOW) {

}

void Chronos::setMonth(uint8_t Month) {

}

void Chronos::setYear(uint8_t Year) {

}

void Chronos::setTime(	uint8_t Hours,
						uint8_t Minutes,
						uint8_t Seconds) {

}

void Chronos::setDate(	uint8_t Day,
						uint8_t Month,
						uint8_t Year) {

}

//**************************** Protected functions *****************************

//***************************** Private functions ******************************
void Chronos::writePosInt(int number) {
    /*
     This function will write a positive integer
     to the display
     */
    if (number < 10000) {
		//If the value is within the range of
		//what can be displayed on the display
		//we execute this piece of code
        uint8_t digit3Value = number % 10;
        number -= digit3Value;
        uint8_t digit2Value = (number % 100) / 10;
        number -= digit2Value;
        uint8_t digit1Value = (number % 1000) / 100;
        number -= digit1Value;
        uint8_t digit0Value = (number % 10000) / 1000;

        writeDigit(digits[digit3Value], 3);
        writeDigit(digits[digit2Value], 2);
        writeDigit(digits[digit1Value], 1);
        writeDigit(digits[digit0Value], 0);
    }

    else {
		//If the value exceeds what can be displayed
		//We display OF (OverFlow)
        writeDigit(0b11111100, 2);
        writeDigit(0b10001110, 3);
    }

}

void Chronos::writeNegInt(int number) {
    /*
     This function will write a negative integer
     to the display
     */

    number *= -1;

    if (number < 1000) {
		//If the value is within the range of
		//what can be displayed on the display
		//we execute this piece of code
        uint8_t digit3Value = number % 10;
        number -= digit3Value;
        uint8_t digit2Value = (number % 100) / 10;
        number -= digit2Value;
        uint8_t digit1Value = (number % 1000) / 100;

        uint8_t digit0Value = 0b00000010;

        writeDigit(digits[digit3Value], 3);
        writeDigit(digits[digit2Value], 2);
        writeDigit(digits[digit1Value], 1);
        writeDigit(digit0Value, 0);
    }

    else {
		//If the value exceeds what can be displayed
		//We display OF (OverFlow)
        writeDigit(0b11111100, 2);
        writeDigit(0b10001110, 3);
    }

}

void Chronos::writePosFl(float number, uint8_t DP) {
	/*
	Switch on where the decimal point needs to be
	*/
	switch (DP){
		case 0:
			//if the decimal point needs to be at place 0
			//we have an integer so we can just call that functions
			writePosInt(number);
			break;
		case 1:
			number *= 10;
			break;
		case 2:
			number *= 100;
			break;
		case 3:
			number *= 1000;
			break;
	}

	int numberi = int(number);

	if (DP > 0)	{
		uint8_t digit3Value = numberi % 10;
		numberi -= digit3Value;
		uint8_t digit2Value = (numberi % 100) / 10;
		numberi -= digit2Value;
		uint8_t digit1Value = (numberi % 1000) / 100;
		numberi -= digit1Value;
		uint8_t digit0Value = (numberi % 10000) / 1000;

		switch (DP)	{
		case 1:
			writeDigit(digits[digit3Value], 3);
			writeDigit(digits[digit2Value] + 1, 2);
			writeDigit(digits[digit1Value], 1);
			writeDigit(digits[digit0Value], 0);
			break;
		case 2:
			writeDigit(digits[digit3Value], 3);
			writeDigit(digits[digit2Value], 2);
			writeDigit(digits[digit1Value] + 1, 1);
			writeDigit(digits[digit0Value], 0);
			break;
		case 3:
			writeDigit(digits[digit3Value], 3);
			writeDigit(digits[digit2Value], 2);
			writeDigit(digits[digit1Value], 1);
			writeDigit(digits[digit0Value] + 1, 0);
			break;
		}
	}
}

void Chronos::writeNegFl(float number, uint8_t DP) {
	/*
	Switch on where the decimal point needs to be
	*/
	switch (DP){
		case 0:
			//if the decimal point needs to be at place 0
			//we have an integer so we can just call that functions
			writeNegInt(number);
			break;
		case 1:
			number *= 10;
			break;
		case 2:
			number *= 100;
			break;
	}

	int numberi = int(number * -1);

	if (DP > 0)	{
		uint8_t digit3Value = numberi % 10;
		numberi -= digit3Value;
		uint8_t digit2Value = (numberi % 100) / 10;
		numberi -= digit2Value;
		uint8_t digit1Value = (numberi % 1000) / 100;

		uint8_t digit0Value = 0b00000010;

		switch (DP)	{
			case 1:
				writeDigit(digits[digit3Value], 3);
				writeDigit(digits[digit2Value] + 1, 2);
				writeDigit(digits[digit1Value], 1);
				writeDigit(digit0Value, 0);
				break;
			case 2:
				writeDigit(digits[digit3Value], 3);
				writeDigit(digits[digit2Value], 2);
				writeDigit(digits[digit1Value] + 1, 1);
				writeDigit(digit0Value, 0);
				break;
			case 3:
				writeDigit(digits[digit3Value], 3);
				writeDigit(digits[digit2Value], 2);
				writeDigit(digits[digit1Value], 1);
				writeDigit(digit0Value + 1, 0);
				break;
		}
	}
}

void Chronos::writePosTemp(float temperature) {
	//this function will show a temperature
	//for now the function is limited to positive
	//temperatures only and Celsius only

	//starting to turn the temperature into an
	//integer to make operations easier and faster
	//to preserve one decimal digit we multiply
	//by 10, later we will manually place
	//the decimal point
	int temp = temperature * 10;
	//same operation as with the integer
	//however we only need 3 digits here
	//the fourth digit will be the Celsius sign
	uint8_t digit2Value = (temp % 10);
	temp -= digit2Value;
	uint8_t digit1Value = (temp % 100) / 10;
	temp -= digit1Value;
	uint8_t digit0Value = (temp % 1000) / 100;

	//here we write all the values to the
	//corresponding digit
	writeDigit(digits[digit0Value], 0);
	//Adding one to the binary data for second
	//digit this will enable the decimal point
	writeDigit(digits[digit1Value] + 1, 1);
	writeDigit(digits[digit2Value], 2);
	//Writing the byte corresponding with
	//a "C" to the display
	writeDigit(0b10011100, 3);
	//turning on the dot for degrees
	writeDigit(0b00100000, 4);
}

void Chronos::writeNegTemp(float temperature) {
	//need to be switched between
	//temperature with a value
	//greater then 10 and smaller then 10

	uint8_t digit1Value;
	uint8_t digit2Value;

	if (temperature >= 9.0) {
		int temp_int = int(temperature);
		digit1Value = (temp_int % 10);
		temp_int -= digit1Value;
		digit2Value = (temp_int % 100) / 10;

		writeDigit(0b00000010, 0);
		writeDigit(digits[digit1Value], 1);
		writeDigit(digits[digit2Value] + 1, 2);
		writeDigit(0b10011100, 3);
		writeDigit(0b00100000, 4);
	}

	else {
		int temp_int = int(temperature * 10);
		digit1Value = (temp_int % 10);
		temp_int -= digit1Value;
		digit2Value = (temp_int % 100) / 10;

		writeDigit(0b00000010, 0);
		writeDigit(digits[digit1Value] + 1, 1);
		writeDigit(digits[digit2Value], 2);
		writeDigit(0b10011100, 3);
		writeDigit(0b00100000, 4);
	}

}

uint8_t Chronos::decToBcd(uint8_t val) {
	/*
	this function will take a byte and
	convert it to a binary coded decimal
	the 4 LSB's are the units
	the 4 MSB's are the decades
	*/
	return ((val / 10 * 16) + (val % 10));
}

uint8_t Chronos::bcdToDec(uint8_t val) {
	/*
	this function will take a binary coded decimal
	number and convert it to a normal byte.
	The BCD is stored in a byte as follows:
	the 4 LSB's are the units
	the 4 MSB's are the decades
	*/
	return ((val / 16 * 10) + (val % 16));
}