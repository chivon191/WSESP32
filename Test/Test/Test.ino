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

#define RXD2 16   // Chân RX của ESP32 (kết nối với TX của AS608)
#define TXD2 17   // Chân TX của ESP32 (kết nối với RX của AS608)

#define PIN_SG90 23 // Output pin used

LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial mySerial(2);  // Serial2 sử dụng TXD2 và RXD2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

PN532_I2C pn532_i2c(Wire);

NfcAdapter nfc = NfcAdapter(pn532_i2c);
String correct_pass = "1234";
String cardId1 = "63 6B 6D 0B";
String tagId1= "E1 B2 99 02";
String tagId = "None";
byte nuidPICC[4];

const byte rows = 4;
const byte columns = 4;

char keys[columns][rows] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};

byte rowPins[rows] = {13, 4, 14, 27};
byte columnPins[columns] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);

uint8_t id;

// Chân GPIO kết nối với loa Piezo
const int piezoPin = 0;
/*const int vibrationPin = 14;  // Chân GPIO kết nối cảm biến rung*/


const char* ssid = "FPT Telecom";
const char* password = "vonvaquoc";


const char* serverName = "192.168.1.5"; 
const int serverPort = 8080; 
int door_stat;

Servo sg90;

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
  pinMode(19, INPUT_PULLUP);
  pinMode(34, INPUT_PULLUP);
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
      
  door_stat = digitalRead(18);
  Serial.println(door_stat);
  readNFC();
    if(tagId == cardId1){
      while(!digitalRead(18))
      {
        sg90.write(90);
      }
      while(digitalRead(18))
      {
        delay(1000);
      }
      sg90.write(0);
      tagId = "";

    }
  
}

bool check_layer1() {
    readNFC();
    if(tagId==cardId1 or tagId==tagId1) return true;
    return false;
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