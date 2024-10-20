#include "MyKeypad.h"
#include "MyBuzzer.h"

MyKeypad::MyKeypad(LiquidCrystal_I2C &lcd_ref) : lcd(lcd_ref), keypad(makeKeymap(keys), rowPins, columnPins, rows, columns) {
    lcd.clear();
}

char MyKeypad::read_character() {
    return keypad.getKey(); // Trả về '\0' nếu không có key nào
}

// int MyKeypad::add_new_password() {
//     String newPass1 = "", newPass2 = "";
//     int attempts = 2; // Số lần nhập lại
//     while (attempts > 0) {
//         String pass = "";
//         int size = 6; // Kích thước mật khẩu
//         lcd.clear();
//         lcd.setCursor(0, 0);
//         lcd.print(attempts == 2 ? "Enter password" : "Re-enter pass");
//         lcd.setCursor(5, 1);

//         while (size) {
//             char key = read_character();
//             delay(100);
//             if (key >= '1' && key <= '9') {
//                 Serial.print(key);
//                 lcd.print("*"); // Hiển thị dấu * thay vì mật khẩu
//                 pass += key;
//                 size--;
//             } else if (key == 'D') {
//                 size = 6; // Reset
//                 pass = "";
//                 lcd.setCursor(0, 1);
//                 lcd.print("                ");
//                 lcd.setCursor(5, 1);
//                 Serial.println("\nReset enter password...");
//             } else if (key == 'A') {
//                 lcd.clear();
//                 lcd.print("Exit");
//                 delay(1000);
//                 return 0;
//             }
//         }

//         if (newPass1.isEmpty()) {
//             newPass1 = pass;
//         } else {
//             newPass2 = pass;
//         }
//         attempts--;
//     }

//     if (newPass1 == newPass2) {
//         correct_pass = newPass1;
//         return 1; // Thành công
//     }
//     return 0; // Thất bại
// }

int MyKeypad::check_password() {
    String pass = "";
    char key = '\0';
    int count = 3; // Số lần thử
    int size = 6; // Kích thước mật khẩu
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("ENTER PASSWORD");
    lcd.setCursor(5, 1);

    while (count > 0) {
        while (size) {
            key = read_character();
            delay(100);
            if (key >= '1' && key <= '9') {
                Serial.print(key);
                lcd.print("*"); // Hiển thị dấu * thay vì mật khẩu
                pass += key;
                size--;
            } else if (key == 'D') {
                size = 6;
                pass = "";
                lcd.setCursor(5, 1);
                lcd.print("      ");
                lcd.setCursor(5, 1);
                Serial.println("\nReset enter password...");
            } else if (key == 'A') {
                return 2; // Thoát
            }
        }

        if (pass == correct_pass) {
            return 1; // Thành công
        } else {
            pass = "";
            count--;
            size = 6;
            if (count > 0) {
                Serial.println("\nFailed...");
                Serial.println("You have " + String(count) + " attempts left.");
                delay(3000); // Thời gian đợi
                lcd.setCursor(5, 1);
                lcd.print("      ");
                lcd.setCursor(5, 1);
            }
        }
    }
    return 0; // Thất bại
}

int MyKeypad::change_password() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Change password");
    delay(1000);
    Serial.println("Enter your password.....");
    if (check_password() == 1) {
        String newPass1 = "", newPass2 = "";
        int attempts = 2; // Số lần nhập lại
        while (attempts > 0) {
            String pass = "";
            int size = 6; // Kích thước mật khẩu
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(attempts == 2 ? "Enter new pass" : "Re-enter pass");
            lcd.setCursor(5, 1);

            while (size) {
                char key = read_character();
                delay(100);
                if (key >= '1' && key <= '9') {
                    Serial.print(key);
                    lcd.print("*"); // Hiển thị dấu * thay vì mật khẩu
                    pass += key;
                    size--;
                } else if (key == 'D') {
                    size = 6; // Reset
                    pass = "";
                    lcd.setCursor(5, 1);
                    lcd.print("      ");
                    lcd.setCursor(5, 1);
                    Serial.println("\nReset enter password...");
                } else if (key == 'A') {
                    lcd.clear();
                    lcd.print("Exit");
                    delay(1000);
                    return 0;
                }
            }

            if (newPass1.isEmpty()) {
                newPass1 = pass;
            } else {
                newPass2 = pass;
            }
            attempts--;
        }

        if (newPass1 == newPass2) {
            correct_pass = newPass1;
            return 1; // Thành công
        }
    }
    return 0; // Thất bại
}

void MyKeypad::menu(MyBuzzer &buzzer) {
    int time = 300; // Thời gian chờ
    char key = '\0';
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1.ChangePass");
    lcd.setCursor(0, 1);
    lcd.print("23.AddDelCard");
    Serial.println("------------------------------------------------");
    Serial.println("MENU. PRESS KEY....");
    Serial.println("1. Change password.");
    Serial.println("2. Add card.");
    Serial.println("3. Delete card.");
    Serial.println("------------------------------------------------");

    while (time > 0) {
        key = read_character();
        delay(100);
        if ((key >= '1' && key <= '3') || key == 'C') break;
        time--;
    }

    if (time == 0) {
        lcd.clear();
        lcd.print("Timeout!"); // Hiển thị thông báo hết thời gian
        buzzer.error();
        delay(1000);
        return; // Thoát
    }

    if (key == '1') {
        if (change_password()) {
            lcd.clear();
            lcd.print("Success");
            delay(1000);
            Serial.println("\nChange password success...");
            buzzer.success();
            return; // Thành công
        } else {
            lcd.clear();
            lcd.print("Failed");
            delay(1000);
            Serial.println("Failed...");
            buzzer.error();
            return; // Thất bại
        }
    } else if (key == '2' || key == '3') {
        lcd.clear();
        lcd.print("Not yet");
        delay(1000);
        Serial.println("Chưa phát triển");
    }
    return; // Thoát
}
