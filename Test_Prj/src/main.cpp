#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
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

#define PIN_SG90 2 // Output pin used
#define sw420Pin 15
#define ledPin 17
#define buzzerPin 0
#define tchFinger 19

LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial mySerial(2);  // Serial2 sử dụng TXD2 và RXD2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// PN532_I2C pn532_i2c(Wire);

// NfcAdapter nfc = NfcAdapter(pn532_i2c);
String correct_pass = "1234";
// String nfcId [] = {"63 6B 6D 0B", "E1 B2 99 02"};
// String tagId = "None";
// byte nuidPICC[4];

const byte rows = 4;
const byte columns = 4;

// int size = sizeof(nfcId)/sizeof(nfcId[0]);

char keys[columns][rows] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};

byte rowPins[rows] = {13, 12, 14, 27};
byte columnPins[columns] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);

uint8_t id = 0;

const char* ssid = "donnn";
const char* password = "1234567890";


const char* serverName = "192.168.1.5"; 
const int serverPort = 8080; 
int door_stat;

Servo sg90;

// void readNFC() {
//   if(nfc.tagPresent())
//   {
//     NfcTag tag = nfc.read();
//     tagId = tag.getUidString();
//     Serial.println("Tag id");
//     Serial.println(tagId);
//     delay(1000);
//   }
// }

// bool checkNFC() {
//   readNFC();
//   for(int i = 0; i < size; i++){
//     if(tagId==nfcId[i]) {
//       lcd.setCursor(2, 1);
//       lcd.print("Valid");
//       delay(2000);
//       lcd.clear();
//       return true;
//     }
//   }
//   lcd.setCursor(2, 1);
//   lcd.print("Not valid");
//   delay(2000);
//   lcd.clear();
//   return false;
// }

// bool addnfc() {
//   readNFC();
//   String temp = tagId;
//   for(int i = 0; i < size; i++){
//     if(tagId==nfcId[i]) {
//       return false;
//     }
//   }
//   for(int i = 0; i < size; i++){
//     if(nfcId[i] == "None") {
//       nfcId[i] = temp;
//       return true;
//     }
//   }
//   nfcId[size] = temp;
//   size += 1;
//   return true;
// }

// bool removenfc() {
//   readNFC();
//   String temp = tagId;
//   for(int i = 0; i < size; i++){
//     if(tagId==nfcId[i]) {
//       nfcId[i] = "None";
//       return true;
//     }
//   }
//   return false;
// }

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
      else if(key == 'A') return 0;
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

bool deleteFingerprint() {
  Serial.println("Vui lòng scan vân tay muốn xóa...");
  for (int i = 0; i < 3; i++) {
    // Chờ đặt tay lên cảm biến
    while (!digitalRead(tchFinger)) {
      Serial.print("Đặt tay lên cảm biến....");
      delay(500);
    }
    
    // Bắt đầu xóa vân tay
    if (finger.getImage() == FINGERPRINT_OK) {
      if (finger.image2Tz() == FINGERPRINT_OK) {
        if (finger.fingerFastSearch() == FINGERPRINT_OK) {
          int id = finger.fingerID; // ID của vân tay
          if (finger.deleteModel(id) == FINGERPRINT_OK) {
            Serial.println("Đã xóa vân tay thành công!");
            return true;
          } else {
            Serial.println("Xóa vân tay thất bại.");
          }
        } else {
          Serial.println("Không tìm thấy vân tay cần xóa.");
        }
      } else {
        Serial.println("Không thể chuyển đổi hình ảnh vân tay.");
      }
    } else {
      Serial.println("Không tìm thấy vân tay.");
    }
  }
  Serial.println("Xóa vân tay thất bại sau 3 lần thử.");
  return false;
}

bool checkFingerprint() {
  Serial.println("Đặt ngón tay lên cảm biến để kiểm tra...");
  for (int i = 0; i < 3; i++) {
    // Chờ đặt tay lên cảm biến
    while (!digitalRead(tchFinger)) {
      Serial.print("Đặt tay lên cảm biến....");
      delay(500);
    }
    
    // Bắt đầu quy trình xác thực vân tay
    if (finger.getImage() == FINGERPRINT_OK) {
      if (finger.image2Tz() == FINGERPRINT_OK) {
        if (finger.fingerFastSearch() == FINGERPRINT_OK) {
          Serial.print("Xác thực vân tay thành công với ID ");
          Serial.println(finger.fingerID);
          return true;
        } else {
          Serial.println("Không tìm thấy vân tay.");
        }
      } else {
        Serial.println("Không thể chuyển đổi hình ảnh vân tay");
      }
    } else {
      Serial.println("Không tìm thấy vân tay.");
    }
    delay(1000);
  }
  Serial.println("Xác thực vân tay thất bại sau 3 lần thử.");
  return false;
}

int getNextAvailableID() {
  for (int i = 1; i < 127; i++)
    if (finger.loadModel(i) != FINGERPRINT_OK) 
      return i;
  return 0;
}

bool enrollFingerprint() {
  int id = getNextAvailableID();
  Serial.print("Đang đăng ký vân tay cho ID: ");
  Serial.println(id);

  for (int i = 1; i <= 2; i++) {
    while (finger.getImage() != FINGERPRINT_OK) {
      Serial.println("Đặt ngón tay lên cảm biến...");
      delay(1000);
    }
    if (finger.image2Tz(i) != FINGERPRINT_OK) {
      Serial.println("Chuyển đổi hình ảnh thất bại");
      return false;
    }
    Serial.println("Lấy dấu vân tay thành công! Hãy tháo tay.");
    delay(2000);
  }

  if (finger.createModel() == FINGERPRINT_OK) {
    if (finger.storeModel(id) == FINGERPRINT_OK) {
      Serial.println("Đăng ký vân tay thành công!");
      return true;
    }
  }
  Serial.println("Đăng ký vân tay thất bại.");
  return false;
}



// void success() {
//   tone(buzzerPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
//   delay(125); // Đợi 125 ms
//   tone(buzzerPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
// }

// // Hàm báo lỗi
// void error() {
//   tone(buzzerPin, 200, 1000); // Tần số 200 Hz, thời gian 1000 ms
// }

bool check_layer2() {
  lcd.clear();
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

int currentMenu = 0;
int submenu = 0;

void displayMenu() {
  lcd.clear(); // Xóa màn hình LCD trước khi hiển thị nội dung mới
  switch (currentMenu) {
    case 0:
      lcd.print("1.NFC 2.Finger");
      lcd.setCursor(0, 1); // Chuyển xuống dòng thứ 2
      lcd.print("3.Setting");
      break;
    case 2: // Menu Cai Dat
      if (submenu == 0) {
        lcd.print("1.Change Pass");
        lcd.setCursor(0, 1);
        lcd.print("2.NFC 3.Finger");
      } else if (submenu == 1) {
        lcd.print("1: Them van tay");
        lcd.setCursor(0, 1);
        lcd.print("2: Xoa van tay");
      } else if (submenu == 2) {
        lcd.print("1: Them the");
        lcd.setCursor(0, 1);
        lcd.print("2: Xoa the");
      }
      break;
  }
}

void controlLock() {
  while(!digitalRead(18)) sg90.write(90);
  while(digitalRead(18)) delay(1000);
  sg90.write(0);
}

void navigateMenu(char key) {
  switch (currentMenu) {
    case 0:
      if (key == '1') {
        lcd.clear();
        lcd.print("Mo khoa bang NFC");
      } else if (key == '2') {
        lcd.clear();
        if (checkFingerprint() == true) {
          if (check_layer2() == true) {
            controlLock();
          }
        }
        currentMenu = 0;
        displayMenu();
      } else if (key == '3') {
        currentMenu = 2;
        submenu = 0;
        displayMenu();
      }
      break;
    case 1: // Menu Cai Dat
      if (submenu == 0) {
        if (key == '1') {
          lcd.clear();
          lcd.print("Doi mat khau");
        } else if (key == '2') {
          submenu = 2; // Chuyển sang Quản lý vân tay
          displayMenu();
        } else if (key == '3') {
          submenu = 1; // Chuyển sang Quản lý vân tay
          displayMenu();
        }
      } else if (submenu == 1) {
        if (key == '1') {
          lcd.clear();
          lcd.print("Them van tay");
          if (enrollFingerprint() == true) {
            lcd.clear();
            lcd.print("Add finger success");
          } else {
            lcd.clear();
            lcd.print("Error add finger");
          }
        } else if (key == '2') {
          lcd.clear();
          lcd.print("Xoa van tay");
        }
      } else if (submenu == 2) {
        if (key == '1') {
          lcd.clear();
          lcd.print("Them the");
        } else if (key == '2') {
          lcd.clear();
          lcd.print("Xoa the");
        }
      }
      break;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd.clear();
  lcd.init();
  lcd.backlight();
  displayMenu();
  delay(1);
  pinMode(sw420Pin, INPUT);
  pinMode(18, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  sg90.attach(PIN_SG90);
  sg90.write(3);
  // nfc.begin();
  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("\n\nAS608 Fingerprint sensor with add/delete/check");

  // Khởi động cảm biến vân tay
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Cảm biến vân tay đã kết nối thành công!");
  } else {
    Serial.println("Không tìm thấy cảm biến vân tay :(");
    while (1) { delay(1); }
  }
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  // if(check_layer1()){
  //   if(check_layer2()){
  //     while(!digitalRead(18)) sg90.write(90);
  //     while(digitalRead(18)) delay(1000);
  //     sg90.write(0);
  //   }
  // }
  // else {
  //   int value = digitalRead(sw420Pin);
  //   Serial.print(value);
  //   delay(100);
  // }
  char key = keypad.getKey(); 
  if (key) { 
    if (key == 'A') { /
      if (submenu != 0) {
        submenu = 0; 
        displayMenu();
      } else if (currentMenu != 0) {
        currentMenu = 0; 
        displayMenu();
      }
    } else {
      navigateMenu(key); 
    }
  }
}