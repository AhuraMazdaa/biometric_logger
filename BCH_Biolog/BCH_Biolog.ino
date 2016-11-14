/********************************************************************************************/
/*--------------------------------------------------------------------------------------------
  Program:      Biometric Logger for BCH Volunteers

  Version:      1.0.0

  Description:  Arduino based biometric logger for entry &
                exit of bch Volunteers.
  
  Hardware:     Arduino Mega and official Arduino Ethernet
                shield.
                2Gb micro SD card formatted FAT16.
                R305 fingerprint sensor.
                4x4 Membrane Keypad.
                DS3231 RTC for timestamp.
                LCD.
                
  Software:     Arduino IDE 1.6.12
                
  Source Code:  - github link: 
  Date:         04 NOV 2016
  Modified:     04 NOV 2016

  Author:       Nebuchadnezzar-II     nishanthkanala@gmail.com 

  Licenses:     (Fingerprint Library)
                Software License Agreement (BSD License)

                Copyright (c) 2012, Adafruit Industries
                All rights reserved.

                Redistribution and use in source and binary forms, with or without
                modification, are permitted provided that the following conditions are met:
                1. Redistributions of source code must retain the above copyright
                notice, this list of conditions and the following disclaimer.
                2. Redistributions in binary form must reproduce the above copyright
                notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the distribution.
                3. Neither the name of the copyright holders nor the
                names of its contributors may be used to endorse or promote products
                derived from this software without specific prior written permission.

                THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
                EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
                WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
                DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
                DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
                (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
                LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
                ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
                (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
                SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--------------------------------------------------------------------------------------------*/
/********************************************************************************************/

//***************************************Libraries******************************************//
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_Fingerprint.h>
#include <RTClib.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
//*****************************************************************************************//

//************************************* Definitions ***************************************//
#define _Entry_ 1
#define _Exit_ 2
#define _Add_New_ 3
#define _Identify_ 4
#define _delete_ 5
#define _Admin_ 6
//*****************************************************************************************//

//************************************Global Variables*************************************//
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {A0, A1,  A2,  A3}; //connect to the   row  pinouts of the keypad
byte colPins[COLS] = {A4, A5, A11, A12}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

LiquidCrystal lcd(36, 38, 40, 42, 44, 46);

SoftwareSerial mySerial(A9, A8);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


//*****************************************************************************************//

void setup() {
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
  analogWrite(2,60);
  lcd.begin(16, 2);

  finger.begin(9600);
  rtc.begin();
  if (rtc.lostPower()) {
    lcd.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
