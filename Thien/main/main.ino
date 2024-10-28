#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Keypad.h>

const byte rows = 4;
const byte columns = 4;

char keys[rows][columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};

byte icon_unlocked[8]={
  0b01110,
  0b10000,
  0b10000,
  0b11111,
  0b11011,
  0b11011,
  0b11111,
};

byte icon_locked[8]={
  0b01110,
  0b10001,
  0b10001,
  0b11111,
  0b11011,
  0b11011,
  0b11111,
};

byte rowPins[rows] = {13, 4, 14, 27};
byte columnPins[columns] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;

String correct_uid_card[4] = {"636b6d0b", "8197c300", "63e418f7"};
String correct_pass = "123456";
int quantity_uid_card = 3;
int buttonPin = 18;
int buzzerPin = 17;
int servoPin = 2;
bool statusBacklight = false;
int timeSleep = 100;

void setup() {
  Serial.begin(115200);
  nfc.begin();
  servo.attach(servoPin);
  lcd.init();
  lcd.clear();
  lcd.createChar(0, icon_locked);
  lcd.createChar(1, icon_unlocked);
  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  delay(2000);
  Serial.println("Please press button.....");
  Serial.println("-----------------------");
  servo.write(180);
  for(int i=0;i<6;i++)
  {
    digitalWrite(buzzerPin, !digitalRead(buzzerPin));
    delay(200);
  }
  servo.write(0);
}

void message_voice(int mode)
{
  if(mode)
  {
    digitalWrite(buzzerPin, 1);
    delay(50);
    digitalWrite(buzzerPin, 0);
    delay(50);
    digitalWrite(buzzerPin, 1);
    delay(50);
    digitalWrite(buzzerPin, 0);
    delay(50);
    digitalWrite(buzzerPin, 1);
    delay(50);
    digitalWrite(buzzerPin, 0);
    delay(50);
  }
  else
  {
    digitalWrite(buzzerPin, 1);
    delay(100);
    digitalWrite(buzzerPin, 0);
    delay(100);
    digitalWrite(buzzerPin, 1);
    delay(100);
    digitalWrite(buzzerPin, 0);
    delay(100);
  }
}

void display_screen_hello()
{
  byte home1[8] = {
    B00000,
    B00001,
    B00011,
    B00011,
    B00111,
    B01111,
    B01111,
    B11111
  };

  byte home2[8] = {
    B00000,
    B10010,
    B11010,
    B11010,
    B11110,
    B11110,
    B11110,
    B11111
  };

  byte home3[8] = {
    B11111,
    B11111,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100
  };

  byte home4[8] = {
    B11111,
    B11111,
    B11111,
    B10001,
    B10001,
    B10001,
    B11111,
    B11111
  };
  lcd.createChar(2, home1);
  lcd.createChar(3, home2);
  lcd.createChar(4, home3);
  lcd.createChar(5, home4);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(2));
  lcd.setCursor(1, 0);
  lcd.write(byte(3));
  lcd.print(" Welcome home");
  lcd.setCursor(0, 1);
  lcd.write(byte(4));
  lcd.setCursor(1, 1);
  lcd.write(byte(5));
  lcd.print("Have a NiceDay");
}

void display_running(int start, int end, int row = 1, int timeDelay = 350)
{
  byte monster1[8] = {
    B11111,
    B10101,
    B11111,
    B11111,
    B11111,
    B01110,
    B11011,
    B11000
  };

  byte monster2[8] = {
    B11111,
    B10101,
    B11111,
    B11111,
    B11111,
    B01110,
    B11011,
    B00011
  };
  lcd.createChar(6, monster1);
  lcd.createChar(7, monster2);
  int check = 1;
  for(int i=start;i<=end;i++)
  {
    if(check)
    {
      lcd.setCursor(i, row);
      lcd.write(byte(6));
      check = 0;
    }
    else
    {
      lcd.setCursor(i, row);
      lcd.write(byte(7));
      check = 1;
    }
    delay(timeDelay);
    lcd.setCursor(i, row);
    lcd.print(" ");
  }
}

String read_uid_card()
{
  String uidString;
  if(nfc.tagPresent(1000))
  {
    uidString = "";
    NfcTag tag = nfc.read();
    byte uid[7];
    unsigned int uidLength = tag.getUidLength();
    tag.getUid(uid, uidLength);
    for (int i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) {
            uidString += "0";
        }
        uidString += String(uid[i], HEX);
    }
  }else uidString = "timeout";
  return uidString;
}

int find_first_index_empty(String strs[])
{
  for(int i=0;i<3;i++)
    if(strs[i] == "empty")
      return i;
  return -1;
}

bool in_array(String strs[], String str)
{
  if(find_first_index_empty(strs) == 3) return false;
  for(int i=0;i<3;i++)
    if(str == strs[i])
      return true;
  return false;
}

int count_empty_slots(String strs[])
{
  int count=0;
  for(int i=0;i<3;i++)
    if(strs[i] == "empty")
      count++;
  return count;
}

char read_character() {
  char key = keypad.getKey();
  if (key) return key;
  return '\0';
}

String read_password(int size = 6, int time = 300)
{
  int countTime = time;
  String pass;
  while (size) {
    char key = read_character();
    delay(100);
    if(key == '\0') 
    {
      countTime--;
      if(countTime == 0) break;
    }
    else
    {
      countTime = 300;
      if (key >= '0' && key <= '9') {
        Serial.print(key);
        lcd.print("*");
        pass += String(key);
        key = '\0';
        size--;
      } else if (key == 'D') {
        size = 6;
        pass = "";
        lcd.setCursor(4, 1);
        lcd.print("[______]");
        lcd.setCursor(5, 1);
        Serial.println("\nReset enter password...");
        Serial.println("----------------------------");
      }else if (key == 'A') break;
    }
  }
  if(size) return "Exit.";
  return pass;
}

int check_password() {
  String pass = "";
  int countError = 3;
  // lcd.clear();
  // lcd.setCursor(1, 0);
  // lcd.print("ENTER PASSWORD");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.write(byte(0));
  lcd.print("   PASSWORD   ");
  lcd.write(byte(0));
  lcd.setCursor(4, 1);
  lcd.print("[______]");
  lcd.setCursor(5, 1);
  while (countError) {
    pass = read_password();
    if(pass == "Exit.") return 2;
    else delay(500);
    if (pass == correct_pass) break;
    else {
      pass = "";
      countError--;
      if (countError) {
        message_voice(0);
        Serial.println("\nFailed....");
        Serial.println("You have " + String(countError) + " times for enter password.");
        Serial.println("You must wait 3 seconds.");
        Serial.println("-----------------------");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Fail. ");
        lcd.print(countError);
        lcd.print(" times");
        for (int i = 3; i >= 1; i--) {
          lcd.setCursor(0, 1);
          lcd.print("Waiting.");
          delay(200);
          for(int i=0;i<4;i++)
          {
            lcd.print(".");
            delay(200);
          }
          lcd.setCursor(8,1);
          lcd.print("    ");
        }
        Serial.println("Enter password again....");
        Serial.println("----------------------------");
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("ENTER PASSWORD");
        lcd.setCursor(4, 1);
        lcd.print("[______]");
        lcd.setCursor(5, 1);
      }
    }
  }
  if (countError) return 1;
  return 0;
}

int change_password() {
  Serial.println("You need enter password....");
  Serial.println("----------------------------");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("You need");
  lcd.setCursor(0,1);
  lcd.print("enter password");
  delay(2000);
  Serial.println("ENTER PASSWORD");
  int result_check_password = check_password();
  Serial.println();
  Serial.println("----------------------------");
  if(result_check_password == 2) return 2;
  Serial.println("Begging change password....");
  Serial.println("----------------------------");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Change password");
  lcd.setCursor(0,1);
  lcd.print("________________");
  lcd.setCursor(4,1);
  for(int i=0;i<7;i++)
  {
    lcd.print("=");
    delay(200);
  }
  lcd.print(">");
  delay(400);
  if (result_check_password == 1) {
    String newPass1 = "";
    String newPass2 = "";
    int times_enter = 2;
    while (times_enter) {
      String pass = "";
      if (times_enter == 2) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter new pass");
        Serial.println("Enter your new password....");
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Re-enter pass");
        Serial.println("\nRe-enter your new password....");
      }
      lcd.setCursor(4, 1);
      lcd.print("[______]");
      lcd.setCursor(5, 1);
      pass = read_password();
      if(pass == "Exit.") return 2;
      else delay(500);
      if (times_enter == 2) newPass1 = pass;
      else newPass2 = pass;
      times_enter--;
    }
    if (newPass1 == newPass2) {
      correct_pass = newPass1;
      return 1;
    }
  }
  return 0;
}

String add_uid_card(String cards[])
{
  if(count_empty_slots(cards) == 0) return "Overflow.";
  Serial.println("You need enter password");
  Serial.println("----------------------------");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("You need");
  lcd.setCursor(2,1);
  lcd.print("enter password");
  delay(2000);
  Serial.println("ENTER PASSWORD");
  int result_check_password = check_password();
  if(result_check_password == 0) return "Failure.";
  if(result_check_password == 2) return "Exit.";
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Scan new card");
  while(true)
  {
    String card = read_uid_card();
    if(card == "timeout") return "Timeout.";
    if(in_array(cards, card)) return("Has existed.");
    int location = find_first_index_empty(cards);
    cards[location] = card;
    return "Success.";
  }
  return "Failure.";
}

String delete_uid_card(String cards[])
{
  if(count_empty_slots(cards) == 2) return "Cannot delete";
  Serial.println("You need enter password");
  Serial.println("----------------------------");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("You need");
  lcd.setCursor(2,1);
  lcd.print("enter password");
  delay(2000);
  Serial.println("ENTER PASSWORD");
  int result_check_password = check_password();
  Serial.println();
  Serial.println("----------------------------");
  if(result_check_password == 0) return "Failure.";
  if(result_check_password == 2) return "Exit.";
  Serial.println("Scan or choose card");
  Serial.println("----------------------------");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Scan or");
  lcd.setCursor(4,1);
  lcd.print("choose card");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(cards[0]);
  lcd.setCursor(8,0);
  lcd.print(cards[1]);
  lcd.setCursor(0,1);
  lcd.print(cards[2]);
  char your_choice;
  String card = "";
  int countTime = 300;
  while(countTime)
  {
    your_choice = read_character();
    card = read_uid_card();
    if(in_array(cards, card))
    {
      for(int i=0;i<3;i++)
       if(cards[i] == card)
        cards[i] = "empty";
      delay(2000);
      return "Success.";
    }
    if((your_choice >= '1' && your_choice <= '3') || your_choice == 'A') break;
    countTime--;
    delay(100);
  }
  Serial.println("abc");
  if(your_choice == 'A' || countTime == 0) return "Exit.";
  int index;
  switch(your_choice)
  {
    case '1':
      index = 0;
      break;
    case '2':
      index = 1;
      break;
    case '3':
      index = 2;
      break;
  }
  if(cards[index] == "empty") return "Empty.";
  cards[index] = "empty";
  return "Success.";
}

int menu()
{
  lcd.clear();
  Serial.println("Menu");
  Serial.println("1. Add card");
  lcd.setCursor(0,0);
  lcd.print("1/2 +/- NFC");
  Serial.println("2. Delete card");
  lcd.setCursor(0,1);
  lcd.print("3. Change pass");
  Serial.println("3. Change password");
  Serial.println("----------------------------");
  int countTime = 300;
  char your_choice;
  while(countTime)
  {
    your_choice = read_character();
    if(your_choice == 'A' || your_choice >= '1' && your_choice <= '3') break;
    countTime--;
    delay(100);
  }
  Serial.println("Your choice is " + String(your_choice));
  Serial.println("----------------------------");
  if(countTime == 0 || your_choice == 'A') return 2;
  if(your_choice == '1')
  {
    String message = add_uid_card(correct_uid_card);
    lcd.clear();
    lcd.setCursor(0, 0);
    if(message == "Success.")
    {
      lcd.print("Add card " + message);
      Serial.println("Add card " + message);
      Serial.println("----------------------------");
      delay(2000);
      return 1;
    }
    lcd.print(message);
    Serial.println(message);
    Serial.println("----------------------------");
    delay(2000);
  }
  else if(your_choice == '2')
  {
    String message = delete_uid_card(correct_uid_card);
    lcd.clear();
    lcd.setCursor(0,0);
    if(message == "Success.")
    {
      lcd.print("Delete card");
      lcd.setCursor(4,1);
      lcd.print(message);
      Serial.println("Delete card SUCCESS");
      Serial.println("----------------------------");
      delay(2000);
      return 1;
    }
    lcd.print(message);
    Serial.println(message);
    Serial.println("----------------------------");
    delay(2000);
  }else
  {
    int result_change_password = change_password();
    Serial.println();
    Serial.println("----------------------------");
    switch(result_change_password)
    {
      case 1:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Change password");
        lcd.setCursor(4,1);
        lcd.print("SUCCESS");
        Serial.println("Change password SUCCESS");
        Serial.println("----------------------------");
        delay(2000);
        break;
      case 2:
        lcd.clear();
        lcd.print("Exit.");
        Serial.println("Exit");
        Serial.println("----------------------------");
        delay(2000);
        break;
      case 0:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Change password");
        lcd.setCursor(4,1);
        lcd.print("FAILURE");
        Serial.println("Change password FAILURE");
        Serial.println("----------------------------");
        delay(2000);
    }
  }
  return 0;
}

int check_uid_card()
{
  String uid_card;
  int times = 3;
  int start = 0;
  int end = 0;
  while(times)
  {
    uid_card = read_uid_card();
    if(uid_card == "timeout") return 2;
    if(in_array(correct_uid_card, uid_card)) return 1;
    times--;
    if(times)
    {
      digitalWrite(buzzerPin, 1);
      delay(100);
      digitalWrite(buzzerPin, 0);
      delay(100);
      digitalWrite(buzzerPin, 1);
      delay(100);
      digitalWrite(buzzerPin, 0);
      delay(100);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Fail. ");
      lcd.print(times);
      lcd.print(" times");
      for(int i=0;i<3;i++)
      {
        lcd.setCursor(0,1);
        lcd.print("Waiting.");
        delay(200);
        for(int i=0;i<4;i++)
        {
          lcd.print(".");
          delay(200);
        }
        lcd.setCursor(8,1);
        lcd.print("     ");
      }
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("Please scan");
      lcd.setCursor(10,1);
      lcd.print("again");
    }
  }
  return 0;
}

void loop(void) {
  if(!digitalRead(buttonPin))
  {
    timeSleep = 100;
    lcd.setBacklight(HIGH);
    statusBacklight = true;
    display_screen_hello();
    Serial.println("BUTTON PRESS");
    Serial.println("Please realse button....");
    while(!digitalRead(buttonPin))
    {
      delay(500);
    }
    message_voice(1);
    Serial.println("BUTTON REALSE");
    Serial.println("----------------------------");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.write(byte(0));
    lcd.print("  SCAN CARD   ");
    lcd.write(byte(0));
    Serial.println("Please scan card to unlock....");
    Serial.println("----------------------------");
    int result_check_uid_card = check_uid_card();
    int check_class_1 = 0;
    if(result_check_uid_card == 1)
    {
      check_class_1 = 1;
      message_voice(1);
    }
    else if(result_check_uid_card == 0)
    {
      check_class_1 = 0;
      Serial.println("Card incorrect");
      Serial.println("----------------------------");
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Incorrect");
      lcd.setCursor(0,1);
      lcd.print("________________");
      lcd.setCursor(3,1);
      for(int i=0;i<9;i++)
      {
        lcd.print("=");
        digitalWrite(buzzerPin, !digitalRead(buzzerPin));
        delay(200);
      }
      digitalWrite(buzzerPin, 0);
      lcd.print(">");
      delay(400);
    }
    else
    {
      Serial.println("Exit");
      Serial.println("----------------------------");
    }
    int check_class_2 = 0;
    if(check_class_1)
    {
      Serial.println("Enter password to unlock....");
      Serial.println("----------------------------");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter pass");
      lcd.setCursor(5,1);
      lcd.print("to unlock");
      delay(2000);
      Serial.println("ENTER PASSWORD");
      int result_check_password = check_password();
      Serial.println();
      Serial.println("----------------------------");
      if(result_check_password == 1)
      {
        servo.write(180);
        Serial.println("UNLOCK SUCCESS");
        Serial.println("----------------------------");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" UNLOCK SUCCESS");
        message_voice(1);
        lcd.setCursor(1,1);
        lcd.write(byte(0));
        digitalWrite(buzzerPin, 0);
        display_running(3,12);
        lcd.print(" ");
        lcd.write(byte(1));
        delay(1000);
        servo.write(0);
        check_class_2 = 1;
      }else if(result_check_password == 0)
      {
        Serial.println("UNLOCK FAILURE");
        Serial.println("----------------------------");
      }
      else
      {
        Serial.println("Exit");
        Serial.println("----------------------------");
      }
    }
    if(check_class_2)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1. Menu");
      lcd.setCursor(0,1);
      lcd.print("2. Exit");
      Serial.println("Select one option");
      Serial.println("1. Menu");
      Serial.println("2. Exit");
      Serial.println("----------------------------");
      int countTime = 300;
      char your_choice;
      while(countTime)
      {
        your_choice = read_character();
        if(your_choice == 'A' || your_choice == '1' || your_choice == '2') break;
        countTime--;
        delay(100);
      }
      Serial.println("Your choice is " + String(your_choice));
      Serial.println("----------------------------");
      if(your_choice == '1') menu();
    }
    Serial.println("Press button to continue....");
    Serial.println("----------------------------");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Press button");
    lcd.setCursor(5,1);
    lcd.print("to continue");
  }
  if(timeSleep == 30)
  {
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Sleep in 3s");
    byte sleep[8] = {
      B00000,
      B00000,
      B00000,
      B00000,
      B00000,
      B00111,
      B00010,
      B00111
    };
    lcd.createChar(2, sleep);
    lcd.setCursor(11,0);
    lcd.write(byte(2));
  }
  if(timeSleep == 20)
  {
    lcd.setCursor(9,1);
    lcd.print("2");
    byte sleep[8] = {
      B00000,
      B00000,
      B00000,
      B00000,
      B01111,
      B00010,
      B00100,
      B01111
    };
    lcd.createChar(3, sleep);
    lcd.setCursor(12,0);
    lcd.write(byte(3));
  }
  if(timeSleep == 10)
  {
    lcd.setCursor(9,1);
    lcd.print("1");
    byte sleep[8] = {
      B00000,
      B00000,
      B00000,
      B11111,
      B00010,
      B00100,
      B01000,
      B11111
    };
    lcd.createChar(4, sleep);
    lcd.setCursor(13,0);
    lcd.write(byte(4));
  }
  if(statusBacklight == true && timeSleep == 0)
  {
    statusBacklight = false;
    lcd.clear();
    lcd.setBacklight(LOW);
  }
  if(timeSleep < 0) timeSleep = 0;
  else timeSleep--;
  delay(100);
}
