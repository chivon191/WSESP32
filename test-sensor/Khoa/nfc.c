#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
int ledpin = 2;

NfcAdapter nfc = NfcAdapter(pn532_i2c);
String cardId1 = "63 6B 6D 0B";
String tagId1= "E1 B2 99 02";
String tagId = "None";
byte nuidPICC[4];


void setup() {

  Serial.begin(9600);        

  Serial.println('\n');
  pinMode(ledpin,OUTPUT);
  Serial.println("System initialized");

  nfc.begin();

  delay(10);
  
}


void loop() {

  // create some fake data to publish

  readNFC();
  if(tagId==cardId1 or tagId==tagId1)
  {
    digitalWrite(ledpin, HIGH);
    delay(1000);
    digitalWrite(ledpin, LOW);
  }
      

}
void readNFC() {
  if(nfc.tagPresent())
  {
    NfcTag tag = nfc.read();
    tagId = tag.getUidString();
    Serial.println("Tag id");
    Serial.println(tagId);
  }
  delay(1000);
}
