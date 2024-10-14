#include <WiFi.h>
#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
//#include <WebSocketsClient.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

#define PIN_SG90 23 // Output pin used

LiquidCrystal_I2C lcd(0x27, 16, 2);

PN532_I2C pn532_i2c(Wire);

NfcAdapter nfc = NfcAdapter(pn532_i2c);
String correct_pass = "1234";
String nfcId [] = {"63 6B 6D 0B", "E1 B2 99 02"};
String tagId = "None";
byte nuidPICC[4];



// Chân GPIO kết nối với loa Piezo
const int piezoPin = 15;

int door_stat;

Servo sg90;

int size = sizeof(nfcId)/sizeof(nfcId[0]);

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  delay(1);
  lcd.clear();
  lcd.setCursor(0,0);

  lcd.print("   1st Class");
  lcd.setCursor(0,1);
  lcd.print("    Security");
  pinMode(18, INPUT_PULLUP);
  //pinMode(ledPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  //pinMode(vibrationPin, INPUT_PULLUP);
  
  sg90.attach(PIN_SG90);
  sg90.write(3);
  nfc.begin();
  delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available())                                   // if there is data comming
  {
    String command = Serial.readStringUntil('\n');
    if(command == "1")
    {
      Serial.println(size);
      for(int i = 0; i < size; i++)
      Serial.println(nfcId[i]);
    }
    else if(command == "2")
    {
      addnfc();
    }
    else if(command == "3")
    {
      if (check_layer1()) {
        while(!digitalRead(18)) sg90.write(90);
        while(digitalRead(18)) delay(1000);
        sg90.write(0);
      }
    }
  }
}

bool check_layer1() {
  readNFC();
  for(int i = 0; i < size; i++){
    if(tagId==nfcId[i]) return true;
  }
  return false;
}

void addnfc(){
  
  readNFC();
  String temp = tagId;
  nfcId[size] = temp;
  size += 1;
  for(int i = 0; i < size; i++)
      Serial.println(nfcId[i]);
}


void readNFC() {
  if(nfc.tagPresent())
  {
    NfcTag tag = nfc.read();
    tagId = tag.getUidString();
    Serial.println("Tag id");
    Serial.println(tagId);
    delay(1000);
  }
}