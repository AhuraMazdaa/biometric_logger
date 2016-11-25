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
#include <Adafruit_Fingerprint.h>
#include <RTClib.h>
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
#define _year_den 10000 //10000000000
#define _month_den 100 //100000000
#define _date_den 1 //1000000
#define _hours_den 10000
#define _mins_den 100
#define _max_address_ 250
//*****************************************************************************************//

//******************************** Function Definitions ***********************************//
void printToLcd(String s1,String s2);
bool getTime(int &_year,int &_month,int &_date,int &_hours,int &_mins,int &_secs);
//format for setting time is yyyymmddhhmmss 'A' for enter and [B,C,D,*,#] to cancel
void shut_down();
void updateStrings();
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

DateTime datetimenow;
int _year, _month, _date, _hours, _mins, _secs;
String _datetimenow="D123456.CSV";
String dateString="00/00/00";
String timeString="00:00:00";
String _weekday="";

bool _cancel=false;

char mode_key = NULL;

int address = 0;//address of stored fingerprint
byte security = 0;//determines the kind of user

File logs;

//*****************************************************************************************//

void setup() {

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
    //lcd.print("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:

    printToLcd("RTC Lost Power!","lets set the time.");
    
    _cancel=getTime(_year,_month,_date,_hours,_mins,_secs);
    if(!_cancel){
      rtc.adjust(DateTime(_year, _month, _date, _hours, _mins, _secs));
    }
    else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Canceled!");
      //set some default date
      rtc.adjust(DateTime(2016,11,14,6,30,0));
      _cancel=false;
    }
  }

  delay(1);
}

void loop() {
  
  lcd.clear();
  printToLcd("Enter","Mode:");
  mode_key = NULL;
  do{
    mode_key = kpd.getKey();
  }while(mode_key==NULL);

  switch(mode_key){

  case _Entry_:
    //Entry
    printToLcd("Entry","");
    Serial.println("Entry");
    //updateStrings();

    
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
    printToLcd("Set Time in","yyyymmddhhmmss");
    int _year, _month, _date, _hours, _mins, _secs;
    _cancel = getTime(_year,_month,_date,_hours,_mins,_secs);
    if(!_cancel){
      rtc.adjust(DateTime(_year, _month, _date, _hours, _mins, _secs));
    }
    else{
      printToLcd("Cancelled!","");
      //set some default date
      //rtc.adjust(DateTime(2016,11,14,6,30,0));
    printToLcd("Set Time in","yyyy/mm/dd/hh/mm/ss");
    int _year, _month, _date, _hours, _mins, _secs;
    _cancel = getTime(_year,_month,_date,_hours,_mins,_secs);
    if(!_cancel){
      Serial.println(_year);
      Serial.println(_month);
      Serial.println(_date);
      Serial.println(_hours);
      Serial.println(_mins);
      Serial.println(_secs);
      rtc.adjust(DateTime(_year, _month, _date, _hours, _mins, _secs));
    }
    else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Canceled!");
      //set some default date
      rtc.adjust(DateTime(2016,11,14,6,30,0));
    }
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
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(s1);
  lcd.setCursor(0,1);
  lcd.print(s2);
  delay(2000);
}

bool getTime(int &_year,int &_month,int &_date,int &_hours,int &_mins,int &_secs){
  _cancel=false;
  
  lcd.setCursor(0,0);
  lcd.print("Enter Time:");
  lcd.setCursor(0,1);
  //bool cmplt= false;
  int count=0;
  unsigned long num=0;
  byte digit=0;
  char k=0;
  
  while(count<8){
  
    do{
        k=kpd.getKey();
    }while(k==NULL);

    if(k=='A' || k=='B' || k=='C' || k=='D' || k=='*' || k=='#'){
  while(count<14){
    
    char k=kpd.getKey();
    if(k=='A')
      break;
    if(k=='B' || k=='C' || k=='D' || k=='*' || k=='#'){
      _cancel=true;
      return _cancel;
    }
    switch(k){
      case '1':
        digit=1;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '2':
        digit=2;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '3':
        digit=3;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '4':
        digit=4;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '5':
        digit=5;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '6':
        digit=6;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '7':
        digit=7;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '8':
        digit=8;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '9':
        digit=9;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '0':
        digit=0;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      default:
        Serial.println("digit exception!");
        break;
    }
    num=num*10 + digit;
    Serial.println(num);
    count++;
  }

    count++;
  }

  delay(1000);

  if(count<14){
    lcd.clear();
    return true;
  }

  Serial.println(num);
  
  _year=num/_year_den;
  num=num%_year_den;
  _month=num/_month_den;
  num=num%_month_den;
  _date=num/_date_den;
  //num=num/_date_den;

  num=0;
  
  Serial.println(num);

  while(count<14){
  
    do{
        k=kpd.getKey();
    }while(k==NULL);

    if(k=='A' || k=='B' || k=='C' || k=='D' || k=='*' || k=='#'){
      _cancel=true;
      return _cancel;
    }
    switch(k){
      case '1':
        digit=1;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '2':
        digit=2;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '3':
        digit=3;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '4':
        digit=4;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '5':
        digit=5;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '6':
        digit=6;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '7':
        digit=7;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '8':
        digit=8;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '9':
        digit=9;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      case '0':
        digit=0;
        lcd.setCursor(count,1);
        lcd.print(digit);
        break;
      default:
        Serial.println("digit exception!");
        break;
    }
    num=num*10 + digit;
    Serial.println(num);
    count++;
  }

  //Serial.println(num);
  
  _hours=num/_hours_den;
  num=num%_hours_den;
  _mins=num/_mins_den;
  num=num%_mins_den;
  _secs=num;
  printToLcd("New Time","Updated!");
  lcd.clear();

  num=num/_date_den;
  _hours=num/_hours_den;
  num=num/_hours_den;
  _mins=num/_mins_den;
  num=num/_mins_den;
  _secs=num;

  lcd.clear();
  return _cancel;
  }
}

void updateStrings(){
  //readDS3231time(&sec,&minute,&hour,&weekday,&date,&month,&year);
  DateTime now=rtc.now();
  String n="D123456.CSV";//positions 1,2,3,4,5,6
  _year = now.year();
  _month = now.month();
  _date = now.day();
  _weekday.toCharArray(daysOfTheWeek[now.dayOfTheWeek()],12);
  _hours = now.hour();
  _mins = now.minute();
  _secs = now.second();
  
  n[0]='D';
  n[7]='.';
  n[8]='C';
  n[9]='S';
  n[10]='V';
  
  int j=(int(_date)/10)+48;
  n[1]=j;
  j=(int(_date)%10)+48;
  n[2]=j;
  j=(int(_month)/10)+48;
  n[3]=j;
  j=(int(_month)%10)+48;
  n[4]=j;
  j=((int(_year)%100)/10)+48;
  n[5]=j;
  j=(int(_year)%10)+48;
  n[6]=j;
  //n[11]=3;

  n[11]=3;

  _datetimenow=n;

  dateString[0]=(int(_date)/10)+48;
  dateString[1]=(int(_date)%10)+48;
  dateString[3]=(int(_month)/10)+48;
  dateString[4]=(int(_month)%10)+48;
  dateString[6]=((int(_year)%100)/10)+48;
  dateString[7]=(int(_year)%10)+48;

  timeString[0]=(int(_hours)/10)+48;
  timeString[1]=(int(_hours)%10)+48;
  timeString[3]=(int(_mins)/10)+48;
  timeString[4]=(int(_mins)%10)+48;
  timeString[6]=(int(_secs)/10)+48;
  timeString[7]=(int(_secs)%10)+48;

  Serial.print("dateString : ");
  Serial.println(dateString);
  Serial.print("timeString : ");
  Serial.println(timeString);
  Serial.print("_datetimenow : ");
  Serial.println(_datetimenow);
  
}
