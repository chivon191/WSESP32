#include <ESP32Servo.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);

NfcAdapter nfc = NfcAdapter(pn532_i2c);

String cardId1 = "63 6B 6D 0B";
String tagId1= "E1 B2 99 02";
String tagId = "None";
byte nuidPICC[4];

// Chân GPIO kết nối với loa Piezo
const int piezoPin = 15;

#define PIN_SG90 23 // Output pin used

Servo sg90;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(18, INPUT_PULLUP);
  //pinMode(vibrationPin, INPUT_PULLUP);
  sg90.attach(PIN_SG90); // Minimum and maximum pulse width (in µs) to go from 0° to 180
  nfc.begin();
  delay(2000);
}
int pos = 0;
void loop() {
  // put your main code here, to run repeatedly:
  readNFC();
  if(tagId==cardId1)
  {
    while(!digitalRead(18)) 
    {sg90.write(90);}
    while(digitalRead(18)) delay(1000);
    sg90.write(0);
    tagId = "";
  }
  else
  {
    error();
    tagId = "";
  }
}

void success() {
  tone(piezoPin,2700); // Tần số 2700 Hz, thời gian 100 ms
  delay(100);
  noTone(piezoPin);
  delay(125); // Đợi 125 ms
  tone(piezoPin, 2700); // Tần số 2700 Hz, thời gian 100 ms
  delay(100);
  noTone(piezoPin);
}

// Hàm báo lỗi
void error() {
  tone(piezoPin, 200); // Tần số 200 Hz, thời gian 1000 ms
  delay(1000);
  noTone(piezoPin);
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