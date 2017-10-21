#include "Chronos.h"
#include <DS3231.h>


int SER = 9;    //INPUT
int OE = 10;    //blank display on high
int RCLK = 11;  //Latch clock
int SRCLK = 12; //Shift clock
int SRCLR = 13; //RESET on low
int digit0 = 8; //digit select Bit 0 > digits
int digit1 = 7; //digit select Bit 1 > digits
int digit2 = 6; //digit select bit 2 > dots
int sw1 = 2;
int sw2 = 3;
int sw3 = 4;
int sw4 = 5;
unsigned long Times;
int ticker = 0;

int brightness = 1023;

byte digits[] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110, 0b11000000, 0b00000000};
byte digitsDP[] = {0b11111101, 0b01100001, 0b11011011, 0b11110011, 0b01100111, 0b10110111, 0b10111111, 0b11100001, 0b11111111, 0b11110111};

DS3231 rtc(SDA, SCL);
Time t;

void setup() {
  
  // put your setup code here, to run once:
  pinMode(SER, OUTPUT);
  pinMode(OE, OUTPUT);
  digitalWrite(OE, LOW);
  pinMode(RCLK, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(SRCLR, OUTPUT);
  digitalWrite(SRCLR, HIGH);
  
  pinMode(digit0, OUTPUT);
  digitalWrite(digit0, LOW);
  pinMode(digit1, OUTPUT);
  digitalWrite(digit1, LOW);
  pinMode(digit2, OUTPUT);
  digitalWrite(digit2, LOW);

  //brightness = 16;
  rtc.begin();
  Times = millis();
  //setTheTime(11, 27, 30, 10, 07, 2017);
  
}

void loop() { 
  
  // put your main code here, to run repeatedly:
  t = rtc.getTime();
  writeDisplay((t.hour * 100)+(t.min), 4);
  writeDigit(10, 4, LOW);

  //every second the ticker is increased by one
  //without disturbing the display
  if (millis() - Times > 100){
    Times = millis();
    ticker += 1;
  }
  //resetting the ticker when it gets to big
  if(ticker == 10000){
    ticker = 0;
  }
  
}

void setTheTime(byte h, byte m, byte s, byte mh, byte d, byte y){
  rtc.setTime(h, m, s);
  rtc.setDate(mh, d, y);
}

void writeDigit(byte number, byte digit, bool DP){
  
  //for every segment in the display including the display point
  for(byte i = 0; i < 8; i++){
    if(!DP){
      digitalWrite(SER, (digits[number] >> i) & 1);
    }
    else{
      digitalWrite(SER, (digitsDP[number] >> i) & 1);
    }
    digitalWrite(SRCLK, HIGH);
    digitalWrite(SRCLK, LOW);
  }
  //writing the input low
  digitalWrite(SER, LOW);

  switch (digit){
  //setting the digit select pins to light up the 
  //desired digit
    case 0:
      //most left digit
      digitalWrite(digit0, LOW);
      digitalWrite(digit1, LOW);
      digitalWrite(digit2, LOW);
      break;

    case 1:
      digitalWrite(digit0, HIGH);
      digitalWrite(digit1, LOW);
      digitalWrite(digit2, LOW);
      break;

    case 2:
      digitalWrite(digit0, LOW);
      digitalWrite(digit1, HIGH);
      digitalWrite(digit2, LOW);
      break;

    case 3:
      //most right digit
      digitalWrite(digit0, HIGH);
      digitalWrite(digit1, HIGH);
      digitalWrite(digit2, LOW);
      break;
    case 4:
      //time dots
      digitalWrite(digit0, LOW);
      digitalWrite(digit1, LOW);
      digitalWrite(digit2, HIGH);
      break;
      
  }
  //moving the data from the internal register to the output register
  //of the shift register
  digitalWrite(RCLK, HIGH);
  digitalWrite(RCLK, LOW);
  //making the display active again
  digitalWrite(OE, LOW);
  //waiting the appropriate time to have
  //the desired brightness on the display
  delayMicroseconds(brightness);
  //blanking the display
  digitalWrite(OE, HIGH);
}

void writeDisplay(int number, byte DPplace){
  //splitting the number into individual digits
  //for the display  
  byte units = number % 10;
  byte tens = ((number - units) % 100) / 10;
  byte hundreds = ((number - tens - units) % 1000) / 100;
  byte thousands = ((number - hundreds - tens - units)) / 1000;

  //calling the writeDigit function 4 times
  //once for every digit of the number
  bool digitDP0;
  bool digitDP1;
  bool digitDP2;
  bool digitDP3;
  
  switch (DPplace){
    
    case 0:
      digitDP0 = 1;
      digitDP1 = 0;
      digitDP2 = 0;
      digitDP3 = 0;
      break;

   case 1:
      digitDP0 = 0;
      digitDP1 = 1;
      digitDP2 = 0;
      digitDP3 = 0;
      break;

   case 2:
      digitDP0 = 0;
      digitDP1 = 0;
      digitDP2 = 1;
      digitDP3 = 0;
      break;

   case 3:
      digitDP0 = 0;
      digitDP1 = 0;
      digitDP2 = 0;
      digitDP3 = 1;
      break;

   case 4:
      digitDP0 = 0;
      digitDP1 = 0;
      digitDP2 = 0;
      digitDP3 = 0;
      break;

  }
  
  writeDigit(thousands, 0, digitDP0);
  writeDigit(hundreds, 1, digitDP1);
  writeDigit(tens, 2, digitDP2);
  writeDigit(units, 3, digitDP3);
}
