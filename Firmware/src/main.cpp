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
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <esp_sleep.h>
#include <Preferences.h>

#define wakeupPin 4

#define RXD2 16   // Chân RX của ESP32 (kết nối với TX của AS608)
#define TXD2 17   // Chân TX của ESP32 (kết nối với RX của AS608)

#define PIN_SG90 23 // Output pin used
#define buzzerPin 2
#define vibSensor 35

#define BOTtoken "8018398195:AAEw9i4XVnDLDfUnfiWiYjIBMepotF89Zkw"  // Your Bot Token
#define CHAT_ID "6008983815"

Preferences preferences;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
WebSocketsClient webSocket;  
LiquidCrystal_I2C lcd(0x27, 16, 2);
HardwareSerial mySerial(2);  // Serial2 sử dụng TXD2 và RXD2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

String correct_pass = "1234";
String correct_pass_old = "1234";
String nfcId [] = {"63 6B 6D 0B", "E1 B2 99 02"};
String tagId = "None";
byte nuidPICC[4];

const byte rows = 4;
const byte columns = 4;

int size = sizeof(nfcId)/sizeof(nfcId[0]);

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
const char* serverName = "172.20.10.2"; 
const int serverPort = 8080; 
uint8_t door_stat;
bool vibrationDetected;
unsigned long lastCheckTime = 0;
unsigned long lastActivityTime = 0;
unsigned long currentTime;
String mode = "2FA";
Servo sg90;

void savePreferences() {
  preferences.begin("my-app", false);  // Khởi tạo namespace với chế độ ghi
  preferences.putString("password", correct_pass);
  preferences.putString("mode", mode);
  preferences.end();  // Đóng namespace để giải phóng tài nguyên
  Serial.println("Dữ liệu đã được lưu.");
}

void readPreferences() {
  preferences.begin("my-app", true);  // Khởi tạo namespace với chế độ chỉ đọc
  correct_pass = preferences.getString("password", "1911");  // Giá trị mặc định là "1911"
  mode = preferences.getString("mode", "2FA");               // Giá trị mặc định là "2FA"
  preferences.end();  // Đóng namespace
  Serial.println("Dữ liệu đã được tải:");
  Serial.println("Mật khẩu: " + correct_pass);
  Serial.println("Chế độ: " + mode);
}

void message_voice(uint8_t mode)
{
  if(mode)
  {
    unsigned long startTime = millis();
    while (millis() - startTime < 100) {
      digitalWrite(buzzerPin, HIGH);
      delayMicroseconds(185);
      digitalWrite(buzzerPin, LOW);
      delayMicroseconds(185);
    }

    delay(125); // Đợi 125 ms

  // Lặp lại sóng vuông với tần số 2700 Hz trong 100 ms
    startTime = millis();
    while (millis() - startTime < 100) {
      digitalWrite(buzzerPin, HIGH);
      delayMicroseconds(185);
      digitalWrite(buzzerPin, LOW);
      delayMicroseconds(185);
    }
  }
  else if (mode == 0)
  {
    unsigned long startTime = millis();
    while (millis() - startTime < 1000) {
      digitalWrite(buzzerPin, HIGH);
      delayMicroseconds(2500);
      digitalWrite(buzzerPin, LOW);
      delayMicroseconds(2500);
    }
  }
}

char read_character()
{
  char key = keypad.getKey();
  if (key) return key;
  return '\0';
}

bool virtual_password(String pass) {
  if (pass.length() < correct_pass.length()) return false;
  for (int i=0; i<= pass.length() - correct_pass.length(); i++) {
    bool found = true;
    for (int j=0; j < correct_pass.length(); j++) {
      if (pass[i+j] != correct_pass[j]) {
        found = false;
        break;
      }
    }
    if (found) return true;
  }
  return false;
}

bool check_password() {
  String pass = ""; // Chuỗi mật khẩu nhập vào
  char key = '\0';
  int count = 3; // Số lần thử

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("ENTER PASSWORD");
  lcd.setCursor(5, 1);

  while (count) {
    key = read_character(); // Đọc ký tự từ bàn phím
    delay(1); // Đợi 1 ms

    // Kiểm tra nếu ký tự là số
    if (key != '\0' && key >= '0' && key <= '9') {
      Serial.print(key);
      lcd.print("*");
      pass += String(key); // Thêm ký tự vào mật khẩu
    } 
    // Kiểm tra nếu ký tự là 'D' để đặt lại
    else if (key == 'D') {
      pass = ""; // Đặt lại mật khẩu
      lcd.setCursor(5, 1);
      lcd.print("      "); // Xóa mật khẩu trên màn hình
      lcd.setCursor(5, 1);
      Serial.println("\nReset enter password...");
    } 
    // Kiểm tra nếu ký tự là 'C' để xác nhận
    else if (key == 'C') {
      if (virtual_password(pass)) {
        return true; // Mật khẩu đúng
      } else {
        pass = ""; // Đặt lại mật khẩu
        count--; // Giảm số lần thử
        if (count) {
          lcd.setCursor(1, 0);
          lcd.print("Wrong          ");
          lcd.setCursor(0, 1);
          lcd.print(String(count) + " attempts left.");
          Serial.println("\nFailed........");
          Serial.println("You have " + String(count) + " attempts left.");
          Serial.println("You must wait 3 seconds.");
          delay(3000);
          Serial.println("----------------------------------------------------");
          Serial.println("Enter password again.....");
          lcd.setCursor(1, 0);
          lcd.print("ENTER PASSWORD");
          lcd.setCursor(0, 1);
          lcd.print("                 "); // Xóa mật khẩu trên màn hình
          lcd.setCursor(5, 1);
        }
      }
    } 
    // Kiểm tra nếu ký tự là 'A' để thoát
    else if (key == 'A') {
      return false; // Thoát khỏi hàm
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, "Cảnh báo: Nhập sai mật khẩu quá nhiều lần!", "");
  }
  return false; // Nếu không thành công
}


bool change_password() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Change password");
  delay(1000);
  Serial.println("Enter your password.....");

  if (!check_password()) {
    return false; // Nếu mật khẩu không đúng, trả về false
  }

  String newPass1 = "";
  String newPass2 = "";
  int times_enter = 2;

  while (times_enter) {
    String pass = "";
    int size = 4;

    if (times_enter == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter new pass");
      Serial.println("\nEnter your new password.......");

    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Re-enter pass");
      Serial.println("\nRe-enter your new password......");
    }

    lcd.setCursor(5, 1);
    while (size) {
      char key = read_character();
      delay(1);
      if (key >= '0' && key <= '9') {
          Serial.print(key);
          lcd.print("*");
          pass += String(key);
          size--; // Giảm kích thước
      } else if (key == 'D') {
          size = 4; // Đặt lại kích thước
          pass = ""; // Đặt lại mật khẩu
          lcd.setCursor(5, 1);
          lcd.print("      "); // Xóa mật khẩu trên màn hình
          lcd.setCursor(5, 1);
          Serial.println("\nReset enter password...");
      } else if (key == 'A') {
          lcd.clear();
          lcd.print("Exit");
          delay(1000);
          return false; // Thoát khỏi hàm
      }
    }

    // Ghi nhận mật khẩu mới
    if (times_enter == 2) {
      newPass1 = pass;
    } else {
      newPass2 = pass;
    }
    times_enter--;
  }

  // Kiểm tra nếu 2 mật khẩu mới khớp nhau
  if (newPass1 == newPass2) {
    correct_pass = newPass1; // Gán mật khẩu mới
    return true; // Mật khẩu đã được thay đổi thành công
  }

  return false; // Nếu mật khẩu không khớp
}

bool readNFC() {
  if(nfc.tagPresent())
  {
    NfcTag tag = nfc.read();
    tagId = tag.getUidString();
    Serial.println("Tag id");
    Serial.println(tagId);
    delay(1000);
    return true;
  }
  return false;
}

bool checkNFC() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan your card");
  lcd.setCursor(3,1);
  lcd.print("to unlock");
  if (readNFC()) {
    for(int i = 0; i < size; i++){
      if(tagId==nfcId[i]) {
        lcd.clear();
        lcd.setCursor(5, 0);
        lcd.print("SUCCESS");
        lcd.setCursor(2, 1);
        lcd.print("CHECK CARD");
        message_voice(1);
        delay(1500);
        return true;
      }
    }
  }
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("ERROR");
  lcd.setCursor(2, 1);
  lcd.print("CHECK CARD");
  message_voice(0);
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, "WARNING! FAIL SCAN NFC!", "");
  }
  delay(1500);
  return false;
}

bool addnfc() {
  lcd.clear();
  Serial.println("Enter your password........");
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  if (check_password()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan your card");
    lcd.setCursor(2,1);
    lcd.print("you want add");
    readNFC();
    String temp = tagId;
    for(int i = 0; i < size; i++){
      if(tagId==nfcId[i]) {
        lcd.clear();
        lcd.setCursor(6,0);
        lcd.print("ERROR");
        lcd.setCursor(3, 1);
        lcd.print("SAME CARD");
        delay(1500);
        return false;
      }
    }
    for(int i = 0; i < size; i++){
      if(nfcId[i] == "None") {
        nfcId[i] = temp;
        lcd.clear();
        lcd.setCursor(5, 0);
        lcd.print("SUCCESS");
        lcd.setCursor(4,1);
        lcd.print("ADD CARD");
        delay(1500);
        return true;
      }
    }
    nfcId[size] = temp;
    size += 1;
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("SUCCESS");
    lcd.setCursor(4,1);
    lcd.print("ADD CARD");
    Serial.print("Pass: ");
    Serial.println(correct_pass); 
    delay(1500);
    return true;
    }
  return false;
}

bool removenfc() {
  lcd.setCursor(0, 0);
  lcd.print("Scan your card");
  lcd.setCursor(2,1);
  lcd.print("you want del");
  readNFC();
  String temp = tagId;
  for(int i = 0; i < size; i++){
    if(tagId==nfcId[i]) {
      nfcId[i] = "None";
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("SUCCESS");
      lcd.setCursor(4,1);
      lcd.print("DEL CARD");
      delay(1500);
      return true;
    }
  }
  lcd.clear();
  lcd.setCursor(6,0);
  lcd.print("ERROR");
  lcd.setCursor(2, 1);
  lcd.print("NO FIND CARD");
  delay(1500);
  return false;
}

bool deleteFingerprint() {
  Serial.println("Vui lòng scan vân tay muốn xóa...");
  lcd.setCursor(0, 0);
  lcd.print("Scan your finger");
  lcd.setCursor(0, 1);
  lcd.print("you want delete");
  for (int i = 0; i < 3; i++) {
    unsigned long startTime = millis();
    while (millis() - startTime < 3000) {
      // Bắt đầu xóa vân tay
      if (finger.getImage() == FINGERPRINT_OK) {
        if (finger.image2Tz() == FINGERPRINT_OK) {
          if (finger.fingerFastSearch() == FINGERPRINT_OK) {
            int id = finger.fingerID; // ID của vân tay
            if (finger.deleteModel(id) == FINGERPRINT_OK) {
              Serial.println("Đã xóa vân tay thành công!");
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("DELETE FINGER");
              lcd.setCursor(5, 1);
              lcd.print("SUCCESS");
              delay(1500);
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
  }
  Serial.println("Xóa vân tay thất bại sau 3 lần thử.");
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("DELETE FINGER");
  lcd.setCursor(6, 1);
  lcd.print("ERROR");
  delay(1500);
  return false;
}

bool checkFingerprint() {
  Serial.println("Đặt ngón tay lên cảm biến để kiểm tra...");
  lcd.setCursor(0, 0);
  lcd.print("Scan your finger");
  lcd.setCursor(3,1);
  lcd.print("to unlock");
  for (int i = 0; i < 3; i++) {
    unsigned long startTime = millis();
    while (millis() - startTime < 3000) {
      // Bắt đầu quy trình xác thực vân tay
      if (finger.getImage() == FINGERPRINT_OK) {
          if (finger.image2Tz() == FINGERPRINT_OK) {
              if (finger.fingerFastSearch() == FINGERPRINT_OK) {
                  Serial.print("Xác thực vân tay thành công với ID ");
                  Serial.println(finger.fingerID);
                  lcd.clear();
                  lcd.setCursor(0,0);
                  lcd.print("CHECK FINGER");
                  lcd.setCursor(5, 1);
                  lcd.print("SUCCESS");
                  message_voice(1);
                  delay(1500);
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
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, "WARNING! FAIL SCAN FINGERFRINT!", "");
  }
  Serial.println("Xác thực vân tay thất bại sau 3 lần thử.");
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("CHECK FINGER");
  lcd.setCursor(6, 1);
  lcd.print("ERROR");
  message_voice(0);
  delay(1500);
  return false;
}

int getNextAvailableID() {
  for (int i = 1; i < 127; i++)
    if (finger.loadModel(i) != FINGERPRINT_OK) 
      return i;
  return 0;
}

bool enrollFingerprint() {
  lcd.clear();
  Serial.println("Enter your password........");
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  if (check_password()) {
    int id = getNextAvailableID();
    Serial.print("Đang đăng ký vân tay cho ID: ");
    Serial.println(id);
    lcd.setCursor(0, 0);
    lcd.print("Scan your finger");
    lcd.setCursor(2,1);
    lcd.print("you want add");

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
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print(i);
      lcd.print("ST TIME");
      lcd.setCursor(4, 1);
      lcd.print("SUCCESS");
      delay(1500);
    }

    if (finger.createModel() == FINGERPRINT_OK) {
      if (finger.storeModel(id) == FINGERPRINT_OK) {
        Serial.println("Đăng ký vân tay thành công!");
        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("ADD FINGER");
        lcd.setCursor(4, 1);
        lcd.print("SUCCESS");
        delay(1500);
        return true;
      }
    }
    Serial.println("Đăng ký vân tay thất bại.");
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("ADD FINGER");
    lcd.setCursor(4, 1);
    lcd.print("ERROR");
    delay(1500);
    return false;
  }
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
    message_voice(0);
    return false;
  }
}

int currentMenu = 0;
int submenu = 0;

void controlLock() {
  vibrationDetected = false;
  int timedoorclose = 1, timedooropen = 1;
  message_voice(1);
  while(!digitalRead(18) && timedoorclose<=5) 
  {
    sg90.write(90);
    delay(1000);
    timedoorclose++;
  }
  while(digitalRead(18)) 
  {
    timedooropen++;
    delay(1000);
    while (timedooropen >= 5 && digitalRead(18))
    {
      message_voice(0);
      if (WiFi.status() == WL_CONNECTED) {
        bot.sendMessage(CHAT_ID, "WARNING! DOOR IS OPENING TOO LONG!", "");
      }
    }
  }
  sg90.write(0);
  timedooropen = 1;
  timedoorclose = 1;
  vibrationDetected = true;
  lastActivityTime = millis();
  webSocket.sendTXT("closedoor");
}

void displayMenu() {
  lastActivityTime = millis();
  lcd.clear();
  switch (currentMenu) {
    case 0: // Menu chính
      lcd.print("1.NFC 2.Finger");
      lcd.setCursor(0, 1); // Chuyển xuống dòng thứ 2
      if (mode == "1FA") {
        lcd.print("3.Password");
      } else {
        lcd.print("3.Setting");
      }
      break;

    case 2: // Menu Cài đặt
      if (submenu == 0) { // Menu chính của Cài đặt
        lcd.print("1.Change Pass");
        lcd.setCursor(0, 1);
        lcd.print("2.NFC 3.Finger");
      } else if (submenu == 1) { // Quản lý vân tay
        lcd.print("1: Add Finger");
        lcd.setCursor(0, 1);
        lcd.print("2: Del Finger");
      } else if (submenu == 2) { // Quản lý thẻ NFC
        lcd.print("1: Add Card");
        lcd.setCursor(0, 1);
        lcd.print("2: Del Card");
      }
      break;
  }
}

void navigateMenu(char key) {
  switch (currentMenu) {
    case 0: // Menu chính
      if (key == '1') { // NFC
        if (checkNFC() == true) {
          if (mode == "1FA" || check_layer2() == true) {
            controlLock(); // Mở khóa
            // currentMenu = 0;
            // displayMenu();
          }
        }
        displayMenu(); // Quay lại menu chính
      } else if (key == '2') { // Vân tay
        lcd.clear();
        if (checkFingerprint() == true) {
          if (mode == "1FA" || check_layer2() == true) {
            controlLock(); // Mở khóa
          }
        }
        displayMenu(); // Quay lại menu chính
      } else if (key == '3') {
        if (mode == "1FA") { // Mở khóa bằng mật khẩu
          if (check_password() == true) {
            lcd.clear();
            lcd.setCursor(5, 0);
            lcd.print("SUCCESS");
            controlLock(); // Mở khóa
            displayMenu();
          }
        } else { // Chuyển sang menu Cài đặt
          currentMenu = 2;
          submenu = 0;
          displayMenu();
        }
      }
      break;

    case 2: // Menu Cài đặt
      if (submenu == 0) { // Menu chính của Cài đặt
        if (key == '1') { // Đổi mật khẩu
          lcd.clear();
          if (change_password() == true) {
            lcd.clear();
            lcd.print("SUCCESS");
          } else {
            lcd.clear();
            lcd.print("ERROR");
          }
          delay(2000);
          currentMenu = 0; // Quay lại menu chính
          displayMenu();
        } else if (key == '2') { // Quản lý NFC
          submenu = 2;
          displayMenu();
        } else if (key == '3') { // Quản lý vân tay
          submenu = 1;
          displayMenu();
        }
      } else if (submenu == 1) { // Quản lý vân tay
        if (key == '1') { // Thêm vân tay
          lcd.clear();
          enrollFingerprint();
          // delay(2000);
          currentMenu = 0; // Quay lại menu chính
          displayMenu();
        } else if (key == '2') { // Xóa vân tay
          lcd.clear();
          deleteFingerprint();
          // delay(2000);
          currentMenu = 0; // Quay lại menu chính
          displayMenu();
        }
      } else if (submenu == 2) { // Quản lý NFC
        if (key == '1') { // Thêm thẻ NFC
          lcd.clear();
          addnfc();
          // lcd.print("Add card OK!");
          // delay(2000);
          currentMenu = 0; // Quay lại menu chính
          displayMenu();
        } else if (key == '2') { // Xóa thẻ NFC
          lcd.clear();
          removenfc();
          // lcd.print("Del card OK!");
          // delay(2000);
          currentMenu = 0; // Quay lại menu chính
          displayMenu();
        }
      }
      break;
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket đã ngắt kết nối.");
      break;

    case WStype_CONNECTED:
      Serial.println("Kết nối WebSocket thành công!");
      webSocket.sendTXT("Xin chào từ ESP32!");
      webSocket.sendTXT("Password: " + correct_pass);
      webSocket.sendTXT("Mode: " + mode);  // Gửi chế độ bảo mật hiện tại
      break;

    case WStype_TEXT:
      Serial.printf("Dữ liệu nhận được từ server: %s\n", payload);

      // Xử lý lệnh mở cửa
      if (String((char*)payload) == "opendoor") {
        controlLock();
        // webSocket.sendTXT("closedoor"); // Gửi tín hiệu đã đóng cửa
      }

      // Cập nhật mật khẩu
      if (String((char*)payload).startsWith("Password: ")) {
        correct_pass = String((char*)payload).substring(strlen("Password: "));
        Serial.print("Mật khẩu mới đã được cập nhật:");
        Serial.println(correct_pass);
        savePreferences();
        lastActivityTime = millis();
      }

      // Cập nhật chế độ bảo mật
      if (String((char*)payload).startsWith("Mode: ")) {
        mode = String((char*)payload).substring(strlen("Mode: "));
        Serial.print("Chế độ bảo mật đã được cập nhật:");
        Serial.println(mode);
        savePreferences();
        lastActivityTime = millis();
      }
      break;
  }
}

void check_vibration() {
  if (digitalRead(vibSensor) == HIGH) {
    Serial.println("Vibration threshold reached! Sending alert to Telegram...");
    if (WiFi.status() == WL_CONNECTED) {
      bot.sendMessage(CHAT_ID, "Cảnh báo: Phát hiện rung động mạnh! Có thể đang xảy ra phá khóa.", "");
    }
    message_voice(0);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  readPreferences();
  vibrationDetected = true;
  lcd.clear();
  lcd.init();
  lcd.backlight();
  displayMenu();
  delay(1);
  pinMode(18, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(wakeupPin, INPUT);
  pinMode(vibSensor, INPUT);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Root certificate for Telegram

  sg90.attach(PIN_SG90);
  sg90.write(0);
  nfc.begin();
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

  WiFi.begin(ssid, password);

  webSocket.begin(serverName, serverPort, "/ws/?type=esp32"); 
  webSocket.onEvent(webSocketEvent);

  Serial.println("WebSocket đang chờ kết nối...");
  delay(100);
}

void enterLightSleep() {
  Serial.println("Không có hoạt động trong 30 giây, chuyển sang chế độ Light Sleep...");
  lcd.noBacklight();
  lcd.noDisplay();
  
  WiFi.disconnect(true);
  esp_sleep_enable_ext1_wakeup((1ULL << wakeupPin) | (1ULL << vibSensor), ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_light_sleep_start(); // Vào chế độ Light Sleep
  WiFi.begin(ssid, password);
  Serial.println("Đã thoát Light Sleep!");
  lcd.init();
  lcd.backlight();
  displayMenu(); 
}


void main_function() {
  char key = keypad.getKey();
  if (key) { 
    lastActivityTime = millis(); 
    if (key == 'A') { 
      if (submenu != 0) {
        submenu = 0; 
      } else if (currentMenu != 0) {
        currentMenu = 0; 
      }
      displayMenu(); 
    } else {
      navigateMenu(key); 
    }
  }

  // if (millis() - lastCheckTime >= 1000) { 
  //   lastCheckTime = millis();
    if (!digitalRead(18)) {
      check_vibration(); // Kiểm tra cảm biến rung
    }
  // }

  // Kiểm tra trạng thái không hoạt động
  if (millis() - lastActivityTime >= 30000) {
    enterLightSleep();
  }
}

void loop() { 
  if (WiFi.status() == WL_CONNECTED) {
    webSocket.loop();
    main_function();
  } else {
    static unsigned long lastConnectAttempt = 0;
    if (millis() - lastConnectAttempt >= 5000) { // Thử kết nối lại mỗi 5 giây
      lastConnectAttempt = millis();
      WiFi.begin(ssid, password);
    }
    main_function();
  }
}