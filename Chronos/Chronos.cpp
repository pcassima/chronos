/*
 * Library for the Chronos V1 shield
 * Made by Fenix Computers
 * Library written by Pieter-Jan
 * Version V1.0
 */
#include "Chronos.h"
#include <Arduino.h>

const int SER = 9;    //INPUT
const int OE = 10;    //blank display on high
const int RCLK = 11;  //Latch clock
const int SRCLK = 12; //Shift clock
const int SRCLR = 13; //RESET on low

const int digit0 = 8; //digit select Bit 0 > digits
const int digit1 = 7; //digit select Bit 1 > digits
const int digit2 = 6; //digit select bit 2 > dots

int Brightness = 255;

const byte digits[] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110, 0b11000000, 0b00000000};

Chronos::Chronos(){
    //Anything you need when instantiating your object goes here
    pinMode(SER, OUTPUT);
    pinMode(OE, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(SRCLK, OUTPUT);
    pinMode(SRCLR, OUTPUT);

    pinMode(digit0, OUTPUT);
    pinMode(digit1, OUTPUT);
    pinMode(digit2, OUTPUT);

    digitalWrite(OE, LOW);
    digitalWrite(SRCLR, HIGH);

    digitalWrite(digit0, LOW);
    digitalWrite(digit1, LOW);
    digitalWrite(digit2, LOW);
}

Chronos::~Chronos(){}

void Chronos::writeInt(int number){
    int digit0Value = 0;
    int digit1Value = 0;
    int digit2Value = 0;
    int digit3Value = 0;

    digit3Value = number % 10;
    number = number - digit3Value;
    
    digit2Value = (number % 100) / 10;
    number = number - digit2Value;

    digit1Value = (number % 1000) / 100;
    number = number - digit1Value;

    digit0Value = (number % 10000) / 1000;

    writeDigit(digits[digit0Value],0);
    writeDigit(digits[digit1Value],1);
    writeDigit(digits[digit2Value],2);
    writeDigit(digits[digit3Value],3);

}

void Chronos::shiftOutShape(byte shape){
    for (byte i = 0; i < 8; i++){
        digitalWrite(SER, (shape >> i) & 1);

        digitalWrite(SRCLK, HIGH);
        digitalWrite(SRCLK, LOW);
    }
    digitalWrite(SER, LOW);
}

void Chronos::chooseDigit(byte digit){
    digitalWrite(digit0, digit & 1);
    digitalWrite(digit1, digit & 2);
    digitalWrite(digit2, digit & 4);
}

void Chronos::writeDigit(byte shape, byte digit){
    shiftOutShape(shape);
    chooseDigit(digit);
    digitalWrite(RCLK, HIGH);
    digitalWrite(RCLK, LOW);

    digitalWrite(OE,LOW);
    delayMicroseconds(Brightness);
    digitalWrite(OE, HIGH);
}

void Chronos::setBrightness(int newBrightness){
    Brightness = newBrightness;
    
}
