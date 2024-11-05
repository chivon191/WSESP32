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

#define RXD2 16   // Chân RX của ESP32 (kết nối với TX của AS608)
#define TXD2 17   // Chân TX của ESP32 (kết nối với RX của AS608)

#define PIN_SG90 23 // Output pin used
#define sw420Pin 15
#define buzzerPin 2
#define vibSensor 15

#define BOTtoken "8018398195:AAEw9i4XVnDLDfUnfiWiYjIBMepotF89Zkw"  // Your Bot Token
#define CHAT_ID "6008983815"

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
int door_stat;
bool vibrationDetected = false;

Servo sg90;

void message_voice(int mode)
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
  delay(1500);
  return false;
}

bool addnfc() {
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

char read_character()
{
  char key = keypad.getKey();
  if (key) return key;
  return '\0';
}

bool virtual_password(String pass) {
  if (pass.length() < password.length()) return false;
  for (int i=0; i<= pass.length() - password.length(); i++) {
    bool found = true;
    for (int j=0; j < password.length(); j++) {
      if (pass[i+j] != password[j]) {
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
      lcd.print(key);
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
          Serial.println("\nFailed........");
          Serial.println("You have " + String(count) + " attempts left.");
          Serial.println("You must wait 3 seconds.");
          delay(3000);
          Serial.println("----------------------------------------------------");
          Serial.println("Enter password again.....");
          lcd.setCursor(5, 1);
          lcd.print("      "); // Xóa mật khẩu trên màn hình
          lcd.setCursor(5, 1);
        }
      }
    } 
    // Kiểm tra nếu ký tự là 'A' để thoát
    else if (key == 'A') {
      return false; // Thoát khỏi hàm
    }
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
      if (key >= '1' && key <= '9') {
          Serial.print(key);
          lcd.print(key);
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

void displayMenu() {
  lcd.clear(); // Xóa màn hình LCD trước khi hiển thị nội dung mới
  switch (currentMenu) {
    case 0:
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Wellcome home")
      lcd.setCursor(0, 1);
      lcd.print("Scan finger/card")
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
      bot.sendMessage(CHAT_ID, "WARNING! DOOR IS OPENING TOO LONG!", "");
    }
  }
  sg90.write(0);
  timedooropen = 1;
  timedoorclose = 1;
}

void navigateMenu(char key) {
  switch (currentMenu) {
    case 0:
      // if (key == '1') {
      //   if (checkNFC() == true) {
      //     if (check_layer2() == true) {
      //       controlLock();
      //     }
      //   }
      //   currentMenu = 0;
      //   displayMenu();
      // } else if (key == '2') {
      //   lcd.clear();
      //   if (checkFingerprint() == true) {
      //     if (check_layer2() == true) {       
      //       controlLock();
      //     }
      //   }
      //   currentMenu = 0;
      //   displayMenu();
      // } else if (key == '3') {
      //   currentMenu = 2;
      //   submenu = 0;
      //   displayMenu();
      // }
      // break;
      if (key == 'B') {
        currentMenu = 2;
        submenu = 0;
        displayMenu();
      }
      break;
    case 2: // Menu Cai Dat
      if (submenu == 0) {
        if (key == '1') {
          lcd.clear();
          if (change_password() == 1) {
            lcd.clear();
            lcd.print("SUCCESS");
            delay(2000);
          } else {
            lcd.clear();
            lcd.print("ERROR");
            delay(2000);
          }
          currentMenu = 0;
          displayMenu();
        } else if (key == '2') {
          submenu = 2; // Chuyển sang Quản lý nfc
          displayMenu();
        } else if (key == '3') {
          submenu = 1; // Chuyển sang Quản lý vân tay
          displayMenu();
        }
      } else if (submenu == 1) {
        if (key == '1') {
          lcd.clear();
          if (enrollFingerprint() == true) {
            lcd.clear();
            lcd.print("Add finger success");
          } else {
            lcd.clear();
            lcd.print("Error add finger");
          }
          currentMenu = 0;
          displayMenu();
        } else if (key == '2') {
          lcd.clear();
          if (deleteFingerprint() == true) {
            lcd.clear();
            lcd.print("Del finger success");
          } else {
            lcd.clear();
            lcd.print("Error del finger");
          }
          currentMenu = 0;
          displayMenu();
        }
      } else if (submenu == 2) {
        if (key == '1') {
          addnfc();
          Serial.print("Pass: ");
          Serial.print(correct_pass);
          currentMenu = 0;
          displayMenu();
        } else if (key == '2') {
          removenfc();
          currentMenu = 0;
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
      webSocket.sendTXT("Password:" + correct_pass);
      break;
    case WStype_TEXT:
      Serial.printf("Dữ liệu nhận được từ server: %s\n", payload);
      
      if (String((char*)payload) == "opendoor") {
        Serial.println("OPEN DOOR");
        controlLock();   
        Serial.println("CLOSE DOOR");
        webSocket.sendTXT("closedoor");
      }
      break;
  }
}

void IRAM_ATTR detectsVibration() {
  vibrationDetected = true;
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
  pinMode(buzzerPin, OUTPUT);
  pinMode(vibSensor, INPUT);

  attachInterrupt(digitalPinToInterrupt(vibSensor), detectsVibration, FALLING);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Root certificate for Telegram

  sg90.attach(PIN_SG90);
  sg90.write(3);
  nfc.begin();
  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("\n\nAS608 Fingerprint sensor with add/delete/check");

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

void loop() {
  webSocket.loop();
  char key = keypad.getKey(); 
  if (key) { 
    if (key == 'A') { 
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

  if (nfc.tagPresent()) {
    if (checkNFC() == true) {
      if (check_layer2() == true) {
        controlLock();
      }
    }
    currentMenu = 0;
    displayMenu();
  }

  if (touch == HIGH) {
    if (checkFingerprint() == true) {
      if (check_layer2() == true) {       
        controlLock();
      }
    }
    currentMenu = 0;
    displayMenu();
  }

  if(vibrationDetected) {
    bot.sendMessage(CHAT_ID, "WARNING!!!", "");
    Serial.println("Vibration Detected");
    vibrationDetected = false;
  }
}