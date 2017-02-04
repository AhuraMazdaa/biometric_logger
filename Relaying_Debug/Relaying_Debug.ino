/*************************************************** 
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout 
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to 
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define conf_thresh 70
#define sig 5

byte input1=0x00;
byte input2=0x00;

byte ret_val=0x00;

uint8_t getFingerprintEnroll();
uint8_t deleteFingerprint(int del_add);
uint8_t getFingerprintID();

SoftwareSerial mySerial(11, 10);//Communication with Finger Print Sensor

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  Serial.begin(57600);
  Serial.println("Adafruit finger detect test");
  // set the data rate for the sensor serial port
  finger.begin(57600);

  pinMode(sig,OUTPUT);
  digitalWrite(sig,HIGH);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }

  Wire.begin(5);
  Wire.onReceive(rec);
  Wire.onRequest(req);
  
}

void loop()
{
  if(input1 != 0x00)
  {
      switch(input1){
        case 1:
        {
          //identify (no use for input2)
          //identified byte is sent directly to mySerial2
          digitalWrite(sig,LOW);
          uint8_t p2=finger.getImage();
          Serial.println("Waiting for valid finger...");
          while(p2==FINGERPRINT_NOFINGER)
          {
            p2=finger.getImage();
            if(p2!=FINGERPRINT_NOFINGER){
            getFingerprintID();//stores identified result in ret_val
            }

            long x3 = millis();
            while(millis()<x3+100){}
            
          }
          digitalWrite(sig,HIGH);
          break;
        }
        
        case 2:
        {
          //enroll
          digitalWrite(sig,LOW);
          Serial.println("Ready to enroll a fingerprint!");
          //id = readnumber();
          Serial.print("Enrolling ID #");
          Serial.println(input2,DEC);
          getFingerprintEnroll();//Stores at location input2
          digitalWrite(sig,HIGH);
          break;
        }
        case 3:
        {
          //delete
          digitalWrite(sig,LOW);
          Serial.println("Deleting");
          //id = readnumber();
          Serial.print("Deleting ID # ");Serial.println(input2,DEC);
          deleteFingerprint(input2);
          long x23=millis();
          while(millis()<x23+200){}
          digitalWrite(sig,HIGH);
          break;
        }
        default:
          Serial.println("Entered default!");
          break;
        
      }
  }

  input1 = 0x00;
  input2 = 0x00;
  long x=millis();
  while(millis()<x+200){}
}

void rec(int bytes)
{
  Serial.println("Waiting for input");

  //read from i2c
  input1=Wire.read();
  input2=Wire.read();

  Serial.println("Inputs received : ");
  Serial.println(input1,DEC);
  Serial.println(input2,DEC);
  
}

void req()
{
  Serial.println("Request from master!");
  Serial.print("Sending val : ");
  Serial.println(ret_val);

  Wire.write(ret_val);
  ret_val = 0x00;
}

//done
//ret_val = 1 or 0xFF
uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(input2);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      ret_val=0xFF;
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      ret_val=0xFF;
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      ret_val=0xFF;
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      ret_val=0xFF;
      return p;
    default:
      Serial.println("Unknown error");
      ret_val=0xFF;
      return p;
  }
  
  // Serial.println("Remove finger");

  long x = millis();
  while(millis()<x+500){}
  
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(input2);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      ret_val=0xFF;
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      ret_val=0xFF;
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      ret_val=0xFF;
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      ret_val=0xFF;
      return p;
    default:
      Serial.println("Unknown error");
      ret_val=0xFF;
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(input2);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    ret_val=0xFF;
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    ret_val=0xFE;
    return p;
  } else {
    Serial.println("Unknown error");
    ret_val=0xFD;
    return p;
  }   
  
  Serial.print("ID "); Serial.println(input2);
  p = finger.storeModel(uint16_t(input2));
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    ret_val=0x01;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    ret_val=0xFF;
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    ret_val=0xFF;
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    ret_val=0xFF;
    return p;
  } else {
    Serial.println("Unknown error");
    ret_val=0xFF;
    return p;
  }   
}

//done
//ret_val = 1 or 0 if failed
uint8_t deleteFingerprint(int del_add) {
  uint8_t p = -1;
  
  p = finger.deleteModel(uint16_t(del_add));

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    ret_val=1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    ret_val=0xFF;
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    ret_val=0xFF;
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    ret_val=0xFF;
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    ret_val=0xFF;
    return p;
  }   
}

//done
// ret_val is fingerID or 0xFF
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      ret_val = 0xFF;
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      ret_val = 0xFF;
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      ret_val = 0xFF;
      return p;
    default:
      Serial.println("Unknown error");
      ret_val = 0xFF;
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      ret_val = 0xFF;
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      ret_val = 0xFF;
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      ret_val = 0xFF;
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      ret_val = 0xFF;
      return p;
    default:
      Serial.println("Unknown error");
      ret_val = 0xFF;
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    if(finger.confidence>conf_thresh){
        ret_val = byte(finger.fingerID%251);
        }
    else{
        ret_val = 0xFF;
    }
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    ret_val = 0xFF;
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    ret_val = 0xFE;//254
    return p;
  } else {
    Serial.println("Unknown error");
    ret_val = 0xFD;//253
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.println(finger.fingerID); 
  Serial.print("Confidence = "); Serial.println(finger.confidence);
  
}
