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
String nfcId [] = {"63 6B 6D 0B", "E1 B2 99 02"};
String tagId = "None";
byte nuidPICC[4];

const byte rows = 4;
const byte columns = 4;

int size = sizeof(nfcId)/sizeof(nfcId[0]);

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
const int piezoPin = 15;
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
  if(check_layer1()){
    if(check_layer2()){
      while(!digitalRead(18)) sg90.write(90);
      while(digitalRead(18)) delay(1000);
      sg90.write(0);
    }
  }
}

bool check_layer1() {
  readNFC();
  if(tagId==cardId1 or tagId==tagId1) return true;
  return false;
}

bool check_layer2() {
  Serial.println("Enter your password........");
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  if (check_password())
  {
    lcd.clear();
    lcd.setCursor(5,0);//-----Unlock------
    lcd.print("Unlock");
    lcd.setCursor(4,1);//-----Success-----
    lcd.print("Successs");
    Serial.println("\n------------------------------------------------");
    Serial.println("Unlock success...........");
    Serial.println("------------------------------------------------");
    lcd.clear();
    return true;
  }
  else
  {
    lcd.clear();
    lcd.setCursor(5,0);//-----Unlock------
    lcd.print("Unlock");
    lcd.setCursor(5,1);//-----Failed-----
    lcd.print("Failed");
    Serial.println("\n------------------------------------------------");
    Serial.println("Unlock failed...........");
    Serial.println("------------------------------------------------");
    lcd.clear();
    return false;
  }
}

char read_character()
{
  char key = keypad.getKey();
  if (key) return key;
  return '\0';
}

int check_password()
{
  String pass = "";
  char key = '\0';
  int count = 3;
  int size = 4;
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  lcd.setCursor(5,1);
  while (count)
  {
    while (size)
    {
      key = read_character();
      delay(1);
      if (key != '\0' && key >= '1' && key <= '9')
      {
        Serial.print(key);
        lcd.print(key);
        pass += String(key);
        key = '\0';
        size--;
      }
      else if (key == 'D')
      {
        size = 4;
        pass = "";
        lcd.setCursor(5,1);
        lcd.print("      ");
        lcd.setCursor(5,1);
        Serial.println("\nReset enter password...");
      }
      else if(key == 'A') return 2;
    }
    if (pass == correct_pass) break;
    else
    {
      pass = "";
      count--;
      size = 4;
      if (count)
      {
        Serial.println("\nFailed........");
        Serial.println("You have " + String(count) + " times for enter password.");
        Serial.println("You must wait 3 seconds.");
        delay(3000);
        Serial.println("----------------------------------------------------");
        Serial.println("Enter password again.....");
        lcd.setCursor(5,1);
        lcd.print("      ");
        lcd.setCursor(5,1);
      }
    }
  }
  if (count) return 1;
  return 0;
}

int change_password()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Change password");
  delay(1000);
  Serial.println("Enter your password.....");
  int check = check_password();
  if (check == 1)
  {
    String newPass1 = "";
    String newPass2 = "";
    int times_enter = 2;
    while (times_enter)
    {
      String pass = "";
      int size = 4;
      if (times_enter == 2)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter new pass");
        Serial.println("\nEnter your new password.......");
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Re-enter pass");
        Serial.println("\nRe-enter your new password......");
      }
      lcd.setCursor(5,1);
      while (size)
      {
        char key = read_character();
        delay(1);
        if (key >= '1' && key <= '9')
        {
          Serial.print(key);
          lcd.print(key);
          pass += String(key);
          key = '\0';
          size--;
        }
        else if (key == 'D')
        {
          size = 4;
          pass = "";
          lcd.setCursor(5,1);
          lcd.print("      ");
          lcd.setCursor(5,1);
          Serial.println("\nReset enter password...");
        }
        else if (key == 'A')
        {
          lcd.clear();
          lcd.print("Exit");
          delay(1000);
          return 0;
        }
      }
      if (times_enter == 2) newPass1 = pass;
      else newPass2 = pass;
      times_enter--;
    }
    if (newPass1 == newPass2)
    {
      correct_pass = newPass1;
      return 1;
    }
  }
  return 0;
}

void addnfc() {
  readNFC();
  String temp = tagId;
  nfcId[size] = temp;
  size += 1;
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