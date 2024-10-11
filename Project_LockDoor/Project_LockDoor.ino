#include <WiFi.h>
#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
//#include <WebSocketsClient.h>
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

#define PIN_SG90 23 // Output pin used

LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial mySerial(2);  // Serial2 sử dụng TXD2 và RXD2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

PN532_I2C pn532_i2c(Wire);

NfcAdapter nfc = NfcAdapter(pn532_i2c);
String correct_pass = "1234";
String cardId1 = "63 6B 6D 0B";
String tagId1= "E1 B2 99 02";
String tagId = "None";
byte nuidPICC[4];

const byte rows = 4;
const byte columns = 4;

char keys[columns][rows] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};

byte rowPins[rows] = {13, 4, 14, 27};
byte columnPins[columns] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);

uint8_t id;

// Chân GPIO kết nối với loa Piezo
const int piezoPin = 15;
/*const int vibrationPin = 14;  // Chân GPIO kết nối cảm biến rung*/


const char* ssid = "FPT Telecom";
const char* password = "vonvaquoc";


const char* serverName = "192.168.1.5"; 
const int serverPort = 8080; 
int door_stat;

Servo sg90;

//WebSocketsClient webSocket;  

void setup() {

  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  delay(1);
  lcd.clear();
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  //pinMode(ledPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  //pinMode(vibrationPin, INPUT_PULLUP);
  sg90.setPeriodHertz(50);
  sg90.attach(PIN_SG90, 500, 2400);

  nfc.begin();

  //digitalWrite(ledPin, LOW);

  /*WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  Serial.println("Đã kết nối WiFi.");

  webSocket.begin(serverName, serverPort, "/ws/?type=esp32"); 
  webSocket.onEvent(webSocketEvent);

  Serial.println("WebSocket đang chờ kết nối...");
  */
  /*mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);

  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Cảm biến vân tay đã kết nối thành công!");
  } else {
    Serial.println("Không tìm thấy cảm biến vân tay :(");
    while (1) { delay(1); }
  }*/

}

int alivePass = 0;
int firstClassSecurity = 0;

void loop() {
  //webSocket.loop();
  readNFC();
  if(tagId==cardId1 or tagId==tagId1)
  {
    lcd.clear();
    firstClassSecurity = 0;
    Serial.println("CHECK 1");
    tagId = "None";
    if(!correct_pass.length())
    {
      Serial.println("No password. Please enter your password........");
      lcd.setCursor(0,0);
      lcd.print("No pass");
      lcd.setCursor(0,1);
      lcd.print("Enter passowrd");
      delay(1);
      if(add_new_password())
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Success");
        delay(1);
        alivePass = 1;
        Serial.println("\nPassword updated successfully..........");
        Serial.println("------------------------------------------------");
      }
      else 
      {
        alivePass = 0;
        lcd.setCursor(0,0);
        lcd.print("Failed");
        delay(1);
        Serial.println("\n------------------------------------------------");
      }
    }
    else if(alivePass || correct_pass.length())
    {
      Serial.println("Enter your password........");
      lcd.setCursor(1,0);
      lcd.print("ENTER PASSWORD");
      int result_check_password = check_password();
      if (result_check_password == 1)
      {
        sg90.write(90);
        digitalWrite(piezoPin,HIGH);
        delay(10);
        digitalWrite(piezoPin,LOW);
        firstClassSecurity = 1;
        lcd.clear();
        lcd.setCursor(5,0);//-----Unlock------
        lcd.print("Unlock");
        lcd.setCursor(4,1);//-----Success-----
        lcd.print("Successs");
        Serial.println("\n------------------------------------------------");
        Serial.println("Unlock success...........");
        Serial.println("------------------------------------------------");
        lcd.clear();
      }
      else if(result_check_password == 0)
      {
        firstClassSecurity = 0;
        lcd.clear();
        lcd.setCursor(5,0);//-----Unlock------
        lcd.print("Unlock");
        lcd.setCursor(5,1);//-----Failed-----
        lcd.print("Failed");
        Serial.println("\n------------------------------------------------");
        Serial.println("Unlock failed...........");
        Serial.println("------------------------------------------------");
        lcd.clear();
      }
      if (firstClassSecurity)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("A to exit");
        lcd.setCursor(0,1);
        lcd.print("B to menu");
        Serial.println("PRESS 'A' KEY TO EXIT or PRESS 'B' KEY TO GO TO MENU");
        while (true)
        {
          char key = '\0';
          while (key != 'A' && key != 'B')
          {
            key = read_character();
            delay(1);
          }
          lcd.clear();
          if (key == 'B')
          {
            menu();
            break;
          }
          else
          {
            lcd.clear();
            lcd.print("Exit");
            delay(1);
            Serial.println("------------------------------------------------");
            sg90.write(0);
            break;
          }
        }
      }
    }
  }
  /*checkFingerprint();

  // Nếu có dữ liệu từ Serial, đọc tùy chọn
  if (Serial.available()) {
    char option = Serial.read();
    switch (option) {
      case '1':
        addFingerprint();
        break;
      case '2':
        deleteFingerprint();
        break;
      default:
        Serial.println("Lựa chọn không hợp lệ.");
        break;
    }
    delay(1); // tránh lặp quá nhanh
  }*/
  
}

char read_character()
{
  char key = keypad.getKey();
  if (key) return key;
  return '\0';
}

int add_new_password()
{
  int size;
  int times_enter = 2;
  String newPass = "";
  String checknewPass = "";
  while(times_enter)
  {
    String pass = "";
    size = 4;
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
      delay(1);
      if(key >= '1' && key <= '9')
      {
        Serial.print(key);
        lcd.print(key);
        pass += key;
        size--;
      }
      else if(key == 'D')
      {
        size = 4;
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
    //if(size) break;
    if(times_enter==2) newPass = pass;
    else checknewPass = pass;
    times_enter--;
  }
  if(!size && newPass == checknewPass)
  {
    correct_pass = newPass;
    return 1;
  }
  return 0;
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
      else if(key == 'A') return 2;
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

int menu()
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
    delay(1);
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

/*void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket đã ngắt kết nối.");
      break;
    case WStype_CONNECTED:
      Serial.println("Kết nối WebSocket thành công!");
      webSocket.sendTXT("Xin chào từ ESP32!");
      break;
    case WStype_TEXT:
      Serial.printf("Dữ liệu nhận được từ server: %s\n", payload);
      
      if (String((char*)payload) == "opendoor") {
        digitalWrite(ledPin, HIGH);
        Serial.println("Đã bật đèn LED");
        delay(3000);
        digitalWrite(ledPin, LOW);
        Serial.println("Đã tắt đèn LED");
        webSocket.sendTXT("closedoor");
      }
      break;
  }
}*/

void success() {
  tone(piezoPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
  delay(125); // Đợi 125 ms
  tone(piezoPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
}

// Hàm báo lỗi
void error() {
  tone(piezoPin, 200, 1000); // Tần số 200 Hz, thời gian 1000 ms
}

void readNFC() {
  if(nfc.tagPresent())
  {
    NfcTag tag = nfc.read();
    tagId = tag.getUidString();
    Serial.println("Tag id");
    Serial.println(tagId);
  }
  delay(1000);
}
/*
void addFingerprint() {
  Serial.println("Nhập ID vân tay muốn lưu (1 - 127):");
  id = readNumber();
  if (id == 0) return;

  Serial.print("Đang thêm vân tay ID #");
  Serial.println(id);

  if (getFingerprintEnroll()) {
    Serial.println("Đã lưu vân tay thành công!");
  } else {
    Serial.println("Lưu vân tay thất bại!");
  }

  // Quay lại menu chính
  Serial.println("Quay lại menu chính...");
}

void deleteFingerprint() {
  Serial.println("Nhập ID vân tay muốn xóa (1 - 127):");
  id = readNumber();
  if (id == 0) return;

  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    Serial.println("Đã xóa vân tay thành công!");
  } else {
    Serial.println("Xóa vân tay thất bại!");
  }
}

void checkFingerprint() {
  Serial.println("Đặt ngón tay lên cảm biến để kiểm tra...");
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    p = finger.image2Tz();
    if (p == FINGERPRINT_OK) {
      p = finger.fingerFastSearch();
      if (p == FINGERPRINT_OK) {
        Serial.print("Đã nhận diện thành công ID #");
        Serial.print(finger.fingerID);
        Serial.print(" với độ tin cậy: ");
        Serial.println(finger.confidence);
      } else {
        Serial.println("Không tìm thấy vân tay!");
      }
    }
  }
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.println("Đặt ngón tay lên cảm biến...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      // Đợi người dùng đặt ngón tay lên cảm biến
      delay(500); // Đợi một chút trước khi kiểm tra lại
    } else if (p == FINGERPRINT_IMAGEFAIL) {
      Serial.println("Lỗi chụp hình ảnh");
      return p;
    } else if (p != FINGERPRINT_OK) {
      Serial.println("Lỗi không xác định");
      return p;
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Lỗi chuyển đổi hình ảnh");
    return p;
  }

  Serial.println("Lấy dấu vân tay lần 1 thành công. Hãy tháo tay.");
  delay(2000);
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  Serial.println("Đặt lại ngón tay lần nữa...");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      // Đợi người dùng đặt ngón tay lên cảm biến
      delay(500); // Đợi một chút trước khi kiểm tra lại
    } else if (p == FINGERPRINT_IMAGEFAIL) {
      Serial.println("Lỗi chụp hình ảnh");
      return p;
    } else if (p != FINGERPRINT_OK) {
      Serial.println("Lỗi không xác định");
      return p;
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Lỗi chuyển đổi hình ảnh lần 2");
    return p;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Không khớp hai lần quét vân tay");
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    return true; // Trả về true nếu lưu thành công
  } else {
    Serial.println("Lưu vân tay thất bại.");
    return false; // Trả về false nếu lưu thất bại
  }
}*/

uint8_t readNumber() {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}