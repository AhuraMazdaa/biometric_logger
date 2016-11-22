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
                
  Source Code:  - github link: https://github.com/AhuraMazdaa/biometric_logger/tree/master/BCH_Biolog
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
#include "Adafruit_Fingerprint.h"
#include "RTClib.h"
#include <Keypad.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
//*****************************************************************************************//

//************************************* Definitions ***************************************//
#define _Entry_ 'A'
#define _Exit_ 'B'
#define _Add_New_ 'C'
#define _Delete_ 'D'
#define _Identify_ '1'
#define _Admin_ '2'
#define _Mem_Left_ '3'
#define _Set_Time_ '4'
#define _Shutdown_ '5'
#define _SS_ 4
//*****************************************************************************************//

//******************************** Function Definitions ***********************************//
void printToLcd(String s1,String s2);
void shut_down();
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

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

LiquidCrystal lcd(36, 38, 40, 42, 44, 46);

SoftwareSerial mySerial(A9, A8);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

DateTime date_set;
DateTime datetimenow;
//*****************************************************************************************//

void setup() {

  pinMode(_SS_,OUTPUT);

  Serial.begin(9600);
  
  pinMode(2,OUTPUT);
  analogWrite(2,60);
  lcd.begin(16, 2);

  lcd.setCursor(0,0);
  lcd.print("System Init()");
  lcd.setCursor(0,1);
  lcd.print("In Progress...");

  finger.begin(9600);
  rtc.begin();
  if (rtc.lostPower()) {
    lcd.println("RTC lost power, lets set the time!");
    delay(3000);
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  delay(1);
}

void loop() {
  
  lcd.clear();
  char mode_key = kpd.getKey();

  switch(mode_key){

  case _Entry_:
    //Entry
    //Serial.println("Entry");
    printToLcd("Entry","");
    
    break;

  case _Exit_:
    //Exit
    //Serial.println("Exit");
    break;

  case _Add_New_:
    //Add new Vol
    //Serial.println("New Vol");
    break;

  case _Delete_:
    //Delete Vol
    //Serial.println("Delete Vol");
    break;

  case _Identify_:
    //Identify Vol
    //Serial.println("Identify Vol");
    break;

  case _Admin_:
    //Make Admin
    //Serial.println("Make Admin");
    break;

  case _Mem_Left_:
    //Memory Left
    //Serial.println("Mem Left");
    break;

  case _Set_Time_:
    //Set Time
    //Serial.println("Set Time");
    break;

  case _Shutdown_:
    //Shutdown
    //Serial.println("Shutdown");
    shut_down();
    break;

  default:
    //debugging purposes
  
  break;

  }
  
}

void shut_down(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ready to be");
  lcd.setCursor(0,1);
  lcd.print("Powered Off!");
  while(true){}
}

void printToLcd(String s1,String s2){
  lcd.setCursor(0,0);
  lcd.print(s1);
  lcd.setCursor(0,1);
  lcd.print(s2);
  delay(1000);
}
