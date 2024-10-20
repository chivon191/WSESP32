#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include "MyBuzzer.h"

extern String correct_pass; 

class MyKeypad {
    private:
        LiquidCrystal_I2C &lcd;
        Keypad keypad;
        static const byte rows = 4;
        static const byte columns = 4;

        char keys[rows][columns] = {
            {'1', '2', '3', 'A'},
            {'4', '5', '6', 'B'},
            {'7', '8', '9', 'C'},
            {'*', '0', '#', 'D'}
        };

        byte rowPins[rows] = {13, 4, 14, 27};
        byte columnPins[columns] = {26, 25, 33, 32};

    public:
        MyKeypad(LiquidCrystal_I2C &lcd);
        char read_character();
        void menu(MyBuzzer &buzzer);
        // int add_new_password();
        int check_password();
        int change_password();
};
