#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

extern String correct_pass; 

class myKeypad {
    private:
        LiquidCrystal_I2C lcd;
        Keypad keypad;

    public:
        myKeypad();
        char read_character();
        int menu();
        int add_new_password();
        int check_password();
        int change_password();
};