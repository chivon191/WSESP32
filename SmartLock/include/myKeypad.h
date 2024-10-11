#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

class myKeypad {
    private:
        LiquidCrystal_I2C lcd;
        Keypad keypad;
        String correct_pass;
    public:
        myKeypad();
        char read_character();
        int menu();
        int add_new_password();
        int check_password();
        int change_password();
}