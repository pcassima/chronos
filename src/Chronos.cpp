/*
 * Library for the Chronos V1 shield
 * Made by Fenix Computers
 * Library written by Pieter-Jan
 * Version V1.1
 * 14 feb 2018
 */

//including libraries required for datetime datatypes
#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif

//including the header file
#include "Chronos.h"
#include <Arduino.h>

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

//speaks for itself
#define SECONDS_FROM_1970_TO_2000 946684800

//Debug on or off
bool DEBUG_TF = false;

//pins for the 595 shift register
const int SER = 9;    //INPUT
const int OE = 10;    //blank display on high
const int RCLK = 11;  //Latch clock
const int SRCLK = 12; //Shift clock
const int SRCLR = 13; //RESET on low

//pins for the multiplexer ship
const int digit0 = 8; //digit select Bit 0 > digits
const int digit1 = 7; //digit select Bit 1 > digits
const int digit2 = 6; //digit select bit 2 > dots

//initial brightness for the display (maximum)
uint8_t Brightness = 255;

//list with all the bytes needed for display numbers
//onto the 7-segment display
//the order for the bytes is as follows:
//B7 B6 B5 B4 B3 B2 B1 B0
//A  B  C  D  E  F  G  DP

//the 7-segment display is arranged as follows:
/*
 *      *---A---*
 *      |       |
 *      F       B
 *      |       |
 *      *---G---*
 *      |       |
 *      E       C
 *      |       |
 *      *---D---*
 *                  DP
 */
 
const uint8_t digits[] PROGMEM = { 0b11111100,
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
                                0b00000000};

//other global variables
bool intRollOver = true;

Chronos::Chronos(bool debug = false){
    //Contructor for the library
    //similar to the void setup in an arduino sketch
    //Declaring all the shift register pins as outputs
    DEBUG_TF = debug;
    if(DEBUG_TF){
        Serial.begin(9600);
        while(!Serial);
        Serial.println("Debuggin is on for Chronos shield");
    }
    pinMode(SER, OUTPUT);
    pinMode(OE, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(SRCLK, OUTPUT);
    pinMode(SRCLR, OUTPUT);
    
    //declaring all the multiplexer pins as output
    pinMode(digit0, OUTPUT);
    pinMode(digit1, OUTPUT);
    pinMode(digit2, OUTPUT);

    if(DEBUG_TF){
        Serial.println("All pins declared");
    }
    
    //setting the pins of the shift register to the
    //required state
    if(DEBUG_TF){
        Serial.println("Setting pins to initial state");
    }
    digitalWrite(OE, LOW);
    digitalWrite(SRCLR, HIGH);
    if(DEBUG_TF){
        Serial.println("SR pins set");
    }
    //same for the multiplexer
    //Default is digit 7 (not present)
    //as a result the display will be blank
    digitalWrite(digit0, HIGH);
    digitalWrite(digit1, HIGH);
    digitalWrite(digit2, HIGH);
    if(DEBUG_TF){
        Serial.println("MUX pins set");
    }
    
    if(DEBUG_TF){
        Serial.println("All pins set");
    }

    //calling this function to initialise the display
    blankDisp();

    if(DEBUG_TF){
        Serial.println("Setup has been completed");
    }
    
}

Chronos::~Chronos(){
    //This function is the deconstructor for our library
    //Here we can place any clean up code
    //at the moment we don't need this
    //but the library structure requires this
    }

void Chronos::setIntRollOver(bool rollOver){
    intRollOver = rollOver;
}

void Chronos::writeInt(int number){
    //this function will write a integer to the display
    //off course the logical maximum is 9999
    //the integer value has to be split into
    //separate numbers for each digit
    
    if (intRollOver && (number > 9999)){
        //by default the number is capped at 9999
        number = 9999;
    }
    else if (!intRollOver && (number > 9999)){
        //here we need to make the display
        //show that the number has overflown
        //still needs to be implemented
    }

    //splitting the number into the digits
    //Via modulo and division
    uint8_t digit3Value = number % 10;
    number = number - digit3Value;    
    uint8_t digit2Value = (number % 100) / 10;
    number = number - digit2Value;
    uint8_t digit1Value = (number % 1000) / 100;
    number = number - digit1Value;
    uint8_t digit0Value = (number % 10000) / 1000;

    //Finally writing all the values to their
    //corresponding digit, via the appropriate function
    writeDigit(digits[digit0Value],0);
    writeDigit(digits[digit1Value],1);
    writeDigit(digits[digit2Value],2);
    writeDigit(digits[digit3Value],3);

}

void Chronos::blankDisp(){
    //this function will put all zero's
    //on the sift registers output
    //creating a blank display
    shiftOutShape(0b00000000);
}

void Chronos::shiftOutShape(uint8_t shape){
    //This function will take a byte
    //and output it to the shift register
    //it has to loop 8 times to shift out
    //all bits
    for (uint8_t i = 0; i < 8; i++){
        //shifting the bits over and
        //using the AND function with 1
        //to get only the least significant bit
        //this one we shift out.
        digitalWrite(SER, (shape >> i) & 1);

        //pulsing the shift clock
        digitalWrite(SRCLK, HIGH);
        digitalWrite(SRCLK, LOW);
    }
    //Writing the input back to a know value
    digitalWrite(SER, LOW);

    //Pulsing the register clock to place the data on the output
    digitalWrite(RCLK, HIGH);
    digitalWrite(RCLK, LOW);
}

void Chronos::chooseDigit(uint8_t digit){
    //this function will write the correct
    //binary code to the multiplexer
    //via the AND function
    //this is comparable to a decimal
    //to binary converter
    //for now a maximum of 8 digits can be used
    digitalWrite(digit0, digit & 1);
    digitalWrite(digit1, digit & 2);
    digitalWrite(digit2, digit & 4);
    //the 4 digits are on 0,1,2,3
    //the time and temperature dots are on 4
}

void Chronos::writeDigit(uint8_t shape, uint8_t digit){
    //This function will write a "shape" to a specific digit
    //every bit in the byte represents one segment in the display,
    //More information can be found in the beginning
    //in the following order:
    //B7 B6 B5 B4 B3 B2 B1 B0
    //A  B  C  D  E  F  G  DP

    //We start with blanking the display
    blankDisp();
    //Then we select the right digit
    chooseDigit(digit);
    //and finally we place the desired shape onto
    //the shift register
    shiftOutShape(shape);    

    //We enable the shift register output
    digitalWrite(OE,LOW);
    //Wait for the desired time
    //The longer we wait the brighter
    //the display will appear to be
    //due to persistance of vision
    delayMicroseconds(Brightness);
    //After waiting the dispable the shift
    //register output again
    digitalWrite(OE, HIGH);
}

void Chronos::setBrightness(uint8_t newBrightness){
    //this function updates the global variable
    //for the brightness to the new desired value
    Brightness = newBrightness;
    
}

void Chronos::showTemp(float temperature){
    //this function will show a temperature
    //for now the function is limited to positive
    //temperatures only and Celcius only

    //starting to turn the temperature into an
    //integer to make operations easier and faster
    //to preserve one decimal digit we multiply
    //by 10, later we will manually place
    //the decimal point
    int temp = (temperature * 10);

    //same operation as with the integer
    //however we only need 3 digits here
    //the fourth digit will be the celcius sign
    uint8_t digit2Value = (temp % 10) / 1;
    temp = temp - digit2Value;
    uint8_t digit1Value = (temp % 100) / 10;
    temp = temp - digit1Value;
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

void Chronos::showTemp(){
    //this function will show the internal
    //temperature of the DS3231
    //for now the function is limited to positive
    //temperatures only and Celcius only

    //starting to turn the temperature into an
    //integer to make operations easier and faster
    //to preserve one decimal digit we multiply
    //by 10, later we will manually place
    //the decimal point
    if(DEBUG_TF){
        Serial.println("displaying temperature from internal register");
    }
    float temperature = getTemperature();
    if(DEBUG_TF){
        Serial.println("Read temperature succesfully");
    }
    int temp = (temperature * 10);

    //same operation as with the integer
    //however we only need 3 digits here
    //the fourth digit will be the celcius sign
    uint8_t digit2Value = (temp % 10) / 1;
    temp = temp - digit2Value;
    uint8_t digit1Value = (temp % 100) / 10;
    temp = temp - digit1Value;
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

void Chronos::showTime(int hours, int minutes){
    //this function will show the time on the display
    //once again splitting the values into values 
    //for the digits
    //first for the hours
    uint8_t hours_2 = hours % 10;
    hours -= hours_2;
    uint8_t hours_1 = (hours % 100) / 10;
    //then for the minuts
    uint8_t minutes_2 = minutes % 10;
    minutes -= minutes_2;
    uint8_t minutes_1 = (minutes % 100) / 10;
    //writing the values to the corresponding digits
    writeDigit(digits[hours_1], 0);
    writeDigit(digits[hours_2], 1);
    writeDigit(digits[minutes_1], 2);
    writeDigit(digits[minutes_2], 3);
    //turning on the time dots
    writeDigit(0b11000000, 4);
}

float Chronos::getTemperature(){
    
    //this function reads to temperature from the
    //DS3231's registers
    //the temperature register get update automatically
    //every 64s
    
    //the vloat value for the final temperature
    float temp3231;
    if(DEBUG_TF){
        Serial.println("Starting TWI communication");
    }
    //a whole bunch of TWI communication stuff
    //first we write the start bit followed by
    //the address of the chip
    Wire.beginTransmission(DS3231_ADDRESS);
    //We write the lowest addres of the temperature register
    //the temperature register is located at 0x11 and 0x12
    //0x11 contains the signed integer portion of the float
    //0x12 contains the fraction portion
    Wire.write(0x11);
    //sending the stop bit
    Wire.endTransmission();
    //the chip now knows which register we want to access
    //We will now request the information of the chip.
    //We need to read 2 bytes from the chip
    //the previous 3 statements can be compared
    //to setting the cursor and then we read from
    //the cursor onward.
    Wire.requestFrom(DS3231_ADDRESS, 2);
    //If we received the information we can read it
    if(Wire.available()){
        //the read function reads one byte at a time
        //so we do this twice
        if(DEBUG_TF){
            Serial.println("Succesfully received information from TWI");
        }
        uint8_t tMSB = Wire.read();
        uint8_t tLSB = Wire.read();

        //reconstructing the float from the two bytes we read
        temp3231 = ((((short)tMSB << 8) | (short)tLSB) >> 6) / 4.0;
    }
    else{
        //if something goes wrong we can return
        //some obvious error value
        //as on the celcius scale the lowest temperature
        //is -273.15 degrees
        temp3231 = -999;
    }
    
    //finishing by return the temperature
    return temp3231;
}

uint8_t Chronos::registerEncode(uint8_t val){    
    return (val / 10 * 16) + (val % 10);
}

uint8_t Chronos::registerDecode(uint8_t val){    
    return (val / 16 * 10) + (val % 16);
}

DateTime::DateTime(uint32_t t) {    
    static const uint8_t daysInMonth [] = {31,28,31,30,31,30,31,31,30,31,30,31};
    t -= SECONDS_FROM_1970_TO_2000;

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;

    uint16_t days = t / 24;
    uint8_t leap;

    for (yOff = 0; ; ++yOff) {
        leap = yOff % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
}

DateTime::DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    if (year >= 2000)
        year -= 2000;
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}
