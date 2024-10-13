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

bool check_layer1() {
    
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
        if (check_layer2 == true) {

        }
        else {

        }
    }
    else {

    }
}