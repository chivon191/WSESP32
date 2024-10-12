#include "myAS608.h"
#include "myKeypad.h"
#include "myBuzzer.h"
#include "myNFC.h"

myAS608 fSensor;
myBuzzer buzzer(0);
myNFC nfc;
myKeypad keypad;

int door_stat;
int alivePass = 0;
int firstClassSecurity = 0;
String correct_pass = "";
String cardId1 = "63 6B 6D 0B";
String tagId1= "E1 B2 99 02";
String tagId = "None";

bool check_layer1() {
    nfc.readNFC();
    tagId = nfc.readID;
    if(tagId==cardId1 or tagId==tagId1) return true;
    return false;
}

bool check_layer2() {

}

void setup() {
    lcd.init();
    lcd.backlight();
    delay(1);
    lcd.clear();
    pinMode(18, INPUT_PULLUP);
    pinMode(19, INPUT_PULLUP);
}

void loop() {
    if (check_layer1 == true) {
        
    }
    else {

    }
}