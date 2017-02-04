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
//#include <Adafruit_Fingerprint.h>
#include <RTClib.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
//#include <SoftwareSerial.h>
//*****************************************************************************************//

//************************************* Definitions ***************************************//
#define _Entry_ 'A'
#define _Exit_ 'B'
#define _Add_New_ 'C'
#define _Delete_ 'D'
#define _Identify_ '1'
//#define _Admin_ '2' //now obsolete
#define _Mem_Left_ '3'
#define _Set_Time_ '4'
#define _Delete_All_ '5'
#define _Shutdown_ '6'
#define _year_den 10000 //10000000000
#define _month_den 100 //100000000
#define _date_den 1 //1000000
#define _hours_den 10000
#define _mins_den 100
#define _max_address_ 250
#define _psswd_add_ 4000
#define busy_pin 32
//*****************************************************************************************//

//******************************** Function Definitions ***********************************//
void printToLcd(String s1,String s2);
bool getTime(int &_year,int &_month,int &_date,int &_hours,int &_mins,int &_secs);
//format for setting time is yyyymmddhhmmss 'A' for enter and [B,C,D,*,#] to cancel
void shut_down(); // just to be sure we dont switch off between a write cycle
String updateStrings(); // Updates _datetimenow , dateString , timeString
bool password(); //returns true if passwd is correct
int free_mem_loc(); //returns minimum free location in EEPROM
void print_occupied_locs();
byte getLocationKpd();
void logIt(char x,byte vol1);
void printDirectory(File dir, int numTabs);
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

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

DateTime datetimenow;
int _year, _month, _date, _hours, _mins, _secs;
char _file_name_[12]="D123456.CSV";
String _weekday; 
bool _cancel=false;

char mode_key = NULL;

int address = 0;//address of stored fingerprint
byte ID = 0;//ID#
int memloc=0;
File logs;
char pass[4]={'1','9','9','2'};
//*****************************************************************************************//

void setup() {

  Serial.begin(9600);
  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  File root = SD.open("/");
  printDirectory(root, 0);
  Serial.println("done!");
  

  pinMode(busy_pin,INPUT);
  pinMode(2,OUTPUT);
  analogWrite(2,60);
  lcd.begin(16, 2);

  lcd.setCursor(0,0);
  lcd.print("System Init()");
  lcd.setCursor(0,1);
  lcd.print("In Progress...");

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
      delay(500);
      //set some default date
      //rtc.adjust(DateTime(2016,11,14,6,30,0));
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
  {
    //Entry
    printToLcd("Entry","");

    //identify vol
    Wire.beginTransmission(5);
    Wire.write(1);
    Wire.write(1);
    Wire.endTransmission();
    delay(100);
    while(digitalRead(busy_pin)!=LOW){}
    while(digitalRead(busy_pin)!=HIGH){}
    //completed identifying
    Wire.requestFrom(5,1);
    byte vol=0;
    vol=Wire.read();
    Serial.print("Vol Idetified as entered is : ");
    Serial.println(vol);
    //begin logging vol
    updateStrings();

    //begin logging into logs file
    Serial.println("logging entry");
    logIt(_Entry_,vol);
    
    break;
  }

  case _Exit_:
  {
    //Exit
    printToLcd("Exit","");
    Serial.println("Exit");
    printToLcd("Place your","Finger");
    //identify person
    Wire.beginTransmission(5);
    Wire.write(1);
    Wire.write(1);
    Wire.endTransmission();
    delay(100);
    while(digitalRead(busy_pin)!=LOW){}
    while(digitalRead(busy_pin)!=HIGH){}
    //completed identifying
    Wire.requestFrom(5,1);
    byte vol=0;
    vol=Wire.read();
    Serial.print("Vol Idetiified as exited is : ");
    Serial.println(vol);

    //begin logging into logs file

    logIt(_Exit_,vol);
    
    break;
  }

  case _Add_New_:
  {
    //Add new Vol
    printToLcd("Add New","Volunteer");
    Serial.println("New Vol");

    byte vol_new = getLocationKpd();

    if(vol_new == -1){
      printToLcd("Adding New","Vol Cancelled");
    }
    else if(vol_new > 250 || vol_new <=0){
      printToLcd("Error Occured","While Adding");
      break;
    }
    else if(EEPROM.read(vol_new)==1){
      printToLcd("Loc already","Occupied!");
      break;
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New Vol Loc =");
    lcd.setCursor(0,1);
    lcd.print(vol_new);
    delay(1000);

    //enroll
    Wire.beginTransmission(5);
    Wire.write(2);
    Wire.write(vol_new);
    Wire.endTransmission();
    delay(100);
    while(digitalRead(busy_pin)!=LOW){}
    while(digitalRead(busy_pin)!=HIGH){}
    //completed enrolling
    Wire.requestFrom(5,1);

    byte response=0;
    response=Wire.read();

    Serial.print("Response received is : ");
    Serial.println(response);

    if(response==1){
      printToLcd("Enrllng New Vol","Success!");
      EEPROM.write(vol_new,1);
    }
    else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Error #");
      lcd.setCursor(0,1);
      lcd.print(response);
    }
    
    break;
  }

  case _Delete_:
  {
    if(password()){
    //Delete Vol
    printToLcd("Delete","Volunteer");
    Serial.println("Delete Vol");

    byte del_vol = getLocationKpd();
    if(del_vol == -1){
      printToLcd("Deleting New","Vol Cancelled");
    }
    else if(del_vol > 250 || del_vol <=0){
      printToLcd("Error Occured","While Deleting");
      break;
    }

    EEPROM.write(del_vol,0);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Del Vol Loc =");
    lcd.setCursor(0,1);
    lcd.print(del_vol);
    delay(1000);

    Wire.beginTransmission(5);
    Wire.write(3);
    Wire.write(del_vol);
    Wire.endTransmission();
    delay(100);
    while(digitalRead(busy_pin)!=LOW){}
    while(digitalRead(busy_pin)!=HIGH){}
    //completed deleting

    Wire.requestFrom(5,1);

    byte response=0;
    response=Wire.read();

    Serial.print("Response received is : ");
    Serial.println(response);

    if(response==1){
      printToLcd("Deleting Vol","Success!");
    }
    else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Error #");
      lcd.setCursor(0,1);
      lcd.print(response);
    }
    }
    else{
      printToLcd("Password 4","Del incorrect");
    }
    
    break;
  }

  case _Identify_:
  {
    //Identify Vol
    //Serial.println("Identify Vol");
    Wire.beginTransmission(5);
    Wire.write(1);
    Wire.write(1);
    Wire.endTransmission();
    delay(100);
    while(digitalRead(busy_pin)!=LOW){}
    while(digitalRead(busy_pin)!=HIGH){}
    //completed identifying
    Wire.requestFrom(5,1);
    byte vol=0;
    vol=Wire.read();

    Serial.print("Vol Identified is : ");
    Serial.println(vol);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Volunteer #");
    lcd.setCursor(0,1);
    lcd.print(vol);

    delay(1000);
    
    break;
  }

  case _Mem_Left_:{
    //Memory Left
    //Serial.println("Mem Left");
    for(int i=1;i<=_max_address_;i++){
      if(EEPROM.read(i)!=0){
        memloc++;
      }
    }
    printToLcd("Memory left =",String(_max_address_ - memloc));
    memloc=0;
    print_occupied_locs();
    break;
  }

  case _Set_Time_:
    //Set Time
    //Serial.println("Set Time");
    printToLcd("Set Time in","yyyymmddhhmmss");
    int _year, _month, _date, _hours, _mins, _secs;
    _cancel = getTime(_year,_month,_date,_hours,_mins,_secs);
    if(!_cancel){
      rtc.adjust(DateTime(_year, _month, _date, _hours, _mins, _secs));

      //Print to Serial Monitor
      Serial.println(_year);
      Serial.println(_month);
      Serial.println(_date);
      Serial.println(_hours);
      Serial.println(_mins);
      Serial.println(_secs);
    }
    else{
      printToLcd("Cancelled!","");
    }
      //set some default date
      //rtc.adjust(DateTime(2016,11,14,6,30,0));
    break;

  case _Delete_All_:{
    if(password()){
      for(byte i=1;i<=_max_address_;i++){
        Wire.beginTransmission(5);
        Wire.write(3);
        Wire.write(i);
        Wire.endTransmission();
        delay(10);
        while(digitalRead(busy_pin)!=LOW){}
        while(digitalRead(busy_pin)!=HIGH){}
        //completed deleting
    
        Wire.requestFrom(5,1);
    
        byte response=0;
        response=Wire.read();

        EEPROM.write(i,0);

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Deleting Vols");
        lcd.setCursor(0,1);
        lcd.print("Progress : ");
        lcd.setCursor(11,1);
        double p = (i / _max_address_) * 100.0;
        lcd.print(p);
        lcd.print('%');
      }
    }
    break;
  }
  
  case _Shutdown_:{
    //Shutdown
    //Serial.println("Shutdown");
    shut_down();
    break;
  }

  default:{
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
  _cancel=true;
  
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
  
  //Serial.println(num);

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

String updateStrings(){
  //readDS3231time(&sec,&minute,&hour,&weekday,&date,&month,&year);
  DateTime now=rtc.now();
  String n="D123456.CSV";//positions 1,2,3,4,5,6
  _year = now.year();
  _month = now.month();
  _date = now.day();
  //_weekday.toCharArray(daysOfTheWeek[now.dayOfTheWeek()],12);
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

  n[11]=0x03;//end of text

  return n;

}

bool password(){
  char pswd[4];
  char c=0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter pswd:");
  lcd.setCursor(0,1);
  for(int i=0;i<4;i++){
    do{
      c=kpd.getKey();
    }while(c==NULL);
    pswd[i]=c;
    lcd.print('*');
  }

  for(int i=0;i<4;i++){
    if(pswd[i]==pass[i]){
      continue;
    }
    else{
      printToLcd("Password","Rejected");
      return false;
    }
  }
  printToLcd("Password","Accepted!");
  return true;
}

int free_mem_loc(){
  int location=-1;
  for(int i=1;i<=_max_address_;i++){
    if(EEPROM.read(i)==0){
      location = i;
      return location;
    }
    else{
      continue;
    }
  }
  return location; //with value -1
}

//for deuging purposes only
void print_occupied_locs(){
  for(int i=0;i<_max_address_;i++){
    if(EEPROM.read(i+1)!=0){
      Serial.print("Memory location ");
      Serial.print(i+1);
      Serial.print(" is occupied. Val = ");
      Serial.println(EEPROM.read(i+1));
    }
    else{
      Serial.print("Memory location ");
      Serial.print(i+1);
      Serial.println(" FREE");
    }
  }
}

byte getLocationKpd(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Entr Loc -");
  int vol_loc=0;
  char k=0;
  byte digit=0;
  int count=0;

  while(count<3){
  
    do{
        k=kpd.getKey();
    }while(k==NULL);

    if(k=='A' || k=='B' || k=='C' || k=='D' || k=='*' || k=='#'){
      return -1;
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
    vol_loc=vol_loc*10 + digit;
    Serial.print("vol_loc = ");
    Serial.println(vol_loc);
    count++;
  }
  if(vol_loc > 250 || vol_loc < 1)
  return -1;
  
  byte ret = (byte)vol_loc;
  return ret;
}

void logIt(char x,byte vol1){
  DateTime now=rtc.now();
  String _fileName = updateStrings();
  Serial.println(_fileName);
  Serial.println(String(daysOfTheWeek[now.dayOfTheWeek()]));
  String _dir_loc_ = "/LOGS/";
  _dir_loc_.concat(_fileName);
  if(!SD.exists(_dir_loc_)){
    Serial.println("created new file");
    logs = SD.open(_dir_loc_,FILE_WRITE);
    logs.println("Volunteer#,Date,Time,Log");
    logs.close();
  }

  logs = SD.open(_dir_loc_);
  logs.print(vol1);
  logs.print(",");
  logs.print(now.day());
  logs.print("/");
  logs.print(now.month());
  logs.print("/");
  logs.print(now.year());
  logs.print(",");
  logs.print(now.hour());
  logs.print(":");
  logs.print(now.minute());
  logs.print(":");
  logs.print(now.second());
  logs.print(",");

  if(x == _Entry_){
    logs.println("entry");
  }
  else{
    logs.println("exit");
  }
  
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
