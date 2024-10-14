#include "MyAS608.h"
#include "MyKeypad.h"
#include "MyBuzzer.h"
#include "MyNFC.h"
#include <ESP32Servo.h>

#define PIN_SG90 23

LiquidCrystal_I2C  lcd(0x27, 16, 2);
MyKeypad keypad(lcd);

MyAS608 vantay(16, 17);
MyBuzzer buzzer(15);
MyNFC nfc;
Servo sg90;

int door_stat;
int alivePass = 0;
int firstClassSecurity = 0;
String correct_pass = "";

bool check_layer1() {
  if (nfc.checkNFC() or vantay.checkFingerprint()) {
    return true;
  }
  return false;
}

bool check_layer2() {
  Serial.println("Enter your password........");
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  if (keypad.check_password())
  {
    lcd.clear();
    lcd.setCursor(5,0); //-----Unlock------
    lcd.print("Unlock");
    lcd.setCursor(4,1); //-----Success-----
    lcd.print("Successs");
    buzzer.success();
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
    buzzer.error();
    Serial.println("\n------------------------------------------------");
    Serial.println("Unlock failed...........");
    Serial.println("------------------------------------------------");
    lcd.clear();
    return false;
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("Hello");
  lcd.init();
  lcd.clear();
  pinMode(18, INPUT_PULLUP);
  // pinMode(ledPin, OUTPUT);
  // pinMode(vibrationPin, INPUT_PULLUP);
  sg90.attach(PIN_SG90);
  sg90.write(3);
  delay(2000);
}

void loop() {
  if (check_layer1()) {
    if (check_layer2()) {
        while(!digitalRead(18)) sg90.write(90);
        while(digitalRead(18)) delay(1000);
        sg90.write(0);
    }
  }
  keypad.menu(buzzer);
}