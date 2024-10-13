#include "myKeypad.h"

myKeypad::myKeypad() : lcd(0x27, 16, 2), keypad(makeKeymap(keys), rowPins, columnPins, rows, columns) {
    const byte rows = 4;
    const byte columns = 4;

    char keys[rows][columns] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    byte rowPins[rows] = {13, 4, 14, 27};
    byte columnPins[columns] = {26, 25, 33, 32};

    lcd.begin(16, 2);
    lcd.backlight();
}

char myKeypad::read_character()
{
  char key = keypad.getKey();
  if (key) return key;
  return '\0';
}

int myKeypad::add_new_password()
{
  int size;
  int times_enter = 2;
  String newPass1 = "";
  String newPass2 = "";
  while(times_enter)
  {
    String pass = "";
    size = 6;
    if(times_enter==2)
    {
      Serial.println("Enter your password......");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter password");
    }
    else
    {
      Serial.println("\nRe-enter your password........");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Re-enter pass");
    }
    lcd.setCursor(5,1);
    while(size)
    {
      char key = read_character();
      delay(100);
      if(key >= '1' && key <= '9')
      {
        Serial.print(key);
        lcd.print(key);
        pass += key;
        size--;
      }
      else if(key == 'D')
      {
        size = 6;
        pass = "";
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(5,1);
        Serial.println("\nReset enter password.........");
      }
      else if(key == 'A')
      {
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print("Exit");
        delay(1000);
        break;
      }
    }
    if(size) break;
    if(times_enter==2) newPass1 = pass;
    else newPass2 = pass;
    times_enter--;
  }
  if(!size && newPass1 == newPass2)
  {
    correct_pass = newPass1;
    return 1;
  }
  return 0;
}

int myKeypad::check_password()
{
  String pass = "";
  char key = '\0';
  int count = 3;
  int size = 6;
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  lcd.setCursor(5,1);
  while (count)
  {
    while (size)
    {
      key = read_character();
      delay(100);
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
        size = 6;
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
      size = 6;
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

int myKeypad::change_password()
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
      int size = 6;
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
        delay(100);
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
          size = 6;
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

int myKeypad::menu()
{
  int time = 300;
  char key = '\0';
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("1.ChangePass");
  lcd.setCursor(0,1);
  lcd.print("23.AddDeleteCard");
  Serial.println("------------------------------------------------");
  Serial.println("MENU. PRESS KEY....");
  Serial.println("1. Change password.");
  Serial.println("2. Add card.");
  Serial.println("3. Delete card.");
  Serial.println("------------------------------------------------");
  while (time)
  {
    key = read_character();
    delay(100);
    if ((key >= '1' && key <= '3') || key == 'C') break;
    time--;
  }
  if (time)
  {
    if (key == '1')
    {
      if (change_password())
      {
        lcd.clear();
        lcd.print("Success");
        delay(1000);
        Serial.println("\nChange password success...........");
        Serial.println("------------------------------------------------");
        return 1;
      }
      else
      {
        lcd.clear();
        lcd.print("Failed");
        delay(1000);
        Serial.println("Failed...........");
        Serial.println("------------------------------------------------");
        return 0;
      }
    }
    else if (key == '2' || key == '3')
    {
      lcd.clear();
      lcd.print("Not yet");
      delay(1000);
      Serial.println("Chua phat trien");
      Serial.println("------------------------------------------------");
    }
  }
  return 0;
}