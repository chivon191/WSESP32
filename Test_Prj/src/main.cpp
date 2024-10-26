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

LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial mySerial(2);  // Serial2 sử dụng TXD2 và RXD2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

PN532_I2C pn532_i2c(Wire);

NfcAdapter nfc = NfcAdapter(pn532_i2c);
String correct_pass = "1234";
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

byte rowPins[rows] = {13, 4, 14, 27};
byte columnPins[columns] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);

uint8_t id = 0;

const char* ssid = "donnn";
const char* password = "1234567890";


const char* serverName = "192.168.1.5"; 
const int serverPort = 8080; 
int door_stat;

Servo sg90;

void readNFC() {
  if(nfc.tagPresent())
  {
    NfcTag tag = nfc.read();
    tagId = tag.getUidString();
    Serial.println("Tag id");
    Serial.println(tagId);
    delay(1000);
  }
}

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

bool addnfc() {
  readNFC();
  for(int i = 0; i < size; i++){
    if(tagId==nfcId[i]) {
      return false;
    }
    else{
      String temp = tagId;
      nfcId[size] = temp;
      size += 1;
      return true;
    }
  }
}

void removenfc() {
  readNFC();
  String temp = tagId;
  nfcId[size] = temp;
  size += 1;
}

uint8_t readNumber() {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

bool getFingerprintEnroll(uint8_t id) {
  Serial.print("Bắt đầu đăng ký vân tay cho ID: ");
  Serial.println(id);
  
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Ảnh vân tay đã được chụp.");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println("Không có ngón tay trên cảm biến.");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Lỗi nhận gói tin.");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Không thể chụp ảnh vân tay.");
        return false;
      default:
        Serial.println("Lỗi không xác định.");
        return false;
    }
  }

  // Tiếp tục xử lý và lưu ảnh vân tay
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Không thể chuyển ảnh thành mẫu vân tay.");
    return false;
  }

  // Đăng ký vân tay với ID
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Tạo mẫu vân tay thành công.");
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
      Serial.println("Lưu mẫu vân tay thành công.");
      return true;
    } else {
      Serial.println("Lưu mẫu vân tay thất bại.");
    }
  } else {
    Serial.println("Tạo mẫu vân tay thất bại.");
  }
  
  return false;
}

bool addFingerprint() {
  id++;
  Serial.println("Bắt đầu thêm vân tay.");
  if (getFingerprintEnroll(id)) {
    Serial.println("Đăng ký vân tay thành công");
    return true;
  } else {
    Serial.println("Đăng ký vân tay thất bại");
    return false;
  }
}

bool deleteFingerprint() {
  int attempts = 0;  // Biến đếm số lần thử quét vân tay
  uint8_t id = 0;    // ID của vân tay cần xóa

  // Yêu cầu người dùng quét vân tay
  while (attempts < 3) {  // Cho phép người dùng thử tối đa 3 lần
    Serial.println("Đặt ngón tay lên cảm biến để xóa...");
    uint8_t p = finger.getImage();

    if (p == FINGERPRINT_OK) {  // Người dùng đã đặt ngón tay lên cảm biến
      attempts++;  // Tính là 1 lần thử

      // Chuyển đổi vân tay thành dữ liệu để kiểm tra
      p = finger.image2Tz();
      if (p == FINGERPRINT_OK) {
        // Tìm kiếm vân tay trong cơ sở dữ liệu
        p = finger.fingerFastSearch();
        if (p == FINGERPRINT_OK) {
          id = finger.fingerID;  // Lưu ID của vân tay được tìm thấy
          Serial.print("Vân tay được phát hiện với ID #");
          Serial.println(id);
          break;  // Thoát khỏi vòng lặp nếu tìm thấy
        } else {
          Serial.println("Không tìm thấy vân tay!");
        }
      }
    } else {
      Serial.println("Lỗi khi đọc vân tay hoặc chưa đặt ngón tay lên cảm biến.");
    }
  }

  // Nếu đã tìm thấy vân tay và ID hợp lệ, tiến hành xóa
  if (id > 0 && attempts <= 3) {
    Serial.print("Xóa vân tay với ID: ");
    Serial.println(id);
    if (finger.deleteModel(id) == FINGERPRINT_OK) {
      Serial.println("Xóa vân tay thành công.");
      return true;
    } else {
      Serial.println("Xóa vân tay thất bại.");
      return false;
    }
  } else {
    Serial.println("Không thể xác định vân tay để xóa.");
    return false;
  }
}

bool checkFingerprint() {
  int attempts = 0;  // Biến đếm số lần thử quét vân tay

  while (attempts < 3) {  // Cho phép người dùng thử tối đa 3 lần
    Serial.println("Đặt ngón tay lên cảm biến để kiểm tra...");
    uint8_t p = finger.getImage();
    
    if (p == FINGERPRINT_OK) {  // Người dùng đã đặt ngón tay lên cảm biến
      attempts++;  // Tính là 1 lần thử
      Serial.print("Số lần thử hiện tại: ");
      Serial.println(attempts);

      p = finger.image2Tz();  // Tiếp tục xử lý vân tay
      if (p == FINGERPRINT_OK) {
        p = finger.fingerFastSearch();
        if (p == FINGERPRINT_OK) {
          Serial.print("Đã nhận diện thành công ID #");
          Serial.print(finger.fingerID);
          Serial.print(" với độ tin cậy: ");
          Serial.println(finger.confidence);
          return true;  // Nhận diện thành công, trả về true
        } else {
          Serial.println("Không tìm thấy vân tay!");

          lcd.setCursor(0,1);
          lcd.print("Wrong!        ");
          delay(3000);
        }
      } 
    } else {
      Serial.println("Lỗi khi đọc vân tay hoặc chưa đặt ngón tay lên cảm biến.");
    }

    // Nếu người dùng không đặt ngón tay hoặc gặp lỗi thì sẽ không tăng attempts
  }

  Serial.println("Bạn đã vượt quá số lần thử vân tay cho phép.");
  return false;  // Nếu quá 3 lần mà vẫn không thành công, trả về false
}



bool Fingerprintmenu(){
  char key_2 = '\0';

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1.Add fingerprint");
  lcd.setCursor(0, 1);
  lcd.print("2.Del fingerprint");
  delay(1000);
  Serial.println("1. Add new fingerprint.");
  Serial.println("2. Delete fingerprint.");

  key_2 = read_character();

  if(key_2 == '1') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan fingerprint");
    if(addFingerprint()) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Add finger successfull");
      return true;
    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Add finger error");
      return false;
    }
  }

  else if(key_2 == '2') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan fingerprint");
    if(deleteFingerprint()) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Delete finger successfull");
      return true;
    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Delete finger error");
      return false;
    }
  }

}

char key = read_character();


int menu() {
  int time = 300;
  char key_1 = '\0';
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("1.ChangePass");
  lcd.setCursor(0,1);
  lcd.print("2.Card  3.Finger");
  Serial.println("------------------------------------------------");
  Serial.println("MENU. PRESS KEY....");
  Serial.println("1. Change password.");
  Serial.println("2. Add/Delete card.");
  Serial.println("3. Add/Delete fingerprint.");
  Serial.println("------------------------------------------------");
  while (time) {
    key_1 = read_character();
    delay(100);
    if ((key_1 >= '1' && key_1 <= '3') || key_1 == 'C') break;
    //time--;
  }
  if (time) {
    if (key_1 == '1') {
      if (change_password()) {
        lcd.clear();
        lcd.print("Success");
        delay(1000);
        Serial.println("\nChange password success...........");
        Serial.println("------------------------------------------------");
        return 1;
      }
      else {
        lcd.clear();
        lcd.print("Failed");
        delay(1000);
        Serial.println("Failed...........");
        Serial.println("------------------------------------------------");
        return 0;
      }
    }
    if (key_1 == '2') {
      lcd.clear();
      Serial.println("1. Add new card.");
      Serial.println("2. Delete card.");
      key = read_character();
      if (key == '1') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scan card");

      }
      if (key == '2') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scan card");
      }
    }
    if (key_1 == '3') {
      if (Fingerprintmenu())
      {
        lcd.clear();
        lcd.print("Success");
        delay(1000);
        Serial.println("\nsuccess...........");
        Serial.println("------------------------------------------------");
        return 1;
      }
      else
      {
        lcd.clear();
        lcd.print("Failed");
        delay(1000);
        Serial.println("\nFailed...........");
        Serial.println("------------------------------------------------");
        return 0;
      }
    }
  }
  return 0;
}

void success() {
  tone(buzzerPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
  delay(125); // Đợi 125 ms
  tone(buzzerPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
}

// Hàm báo lỗi
void error() {
  tone(buzzerPin, 200, 1000); // Tần số 200 Hz, thời gian 1000 ms
}

bool check_layer1() {
  lcd.setCursor(0,0);
  lcd.print("1. Finger");
  lcd.setCursor(0,1);
  lcd.print("2. NFC  3. Menu");
  key = read_character();
  if (key == '1')
  {
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Checking");
    lcd.setCursor(2,1);
    lcd.print("Fingerprint");
    while (true) {
      if(checkFingerprint()) {
        // success();
        return true;
      }
      return false;
    }
  }
  if (key == '2')
  {
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Checking NFC");
    readNFC();
    for(int i = 0; i < size; i++){
      if(tagId==nfcId[i]) {
        // success();
        lcd.setCursor(2, 1);
        lcd.print("Valid");
        delay(1000);
        lcd.clear();
        return true;
      }
    }
    lcd.setCursor(2, 1);
    lcd.print("Not valid");
    delay(2000);
    lcd.clear();
  }
  if(key == '3')
  {
    menu();
  }
  // error();
  return false;
}

bool check_layer2() {
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  delay(1);
  pinMode(sw420Pin, INPUT);
  pinMode(18, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  sg90.attach(PIN_SG90);
  sg90.write(3);
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
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(check_layer1()){
    if(check_layer2()){
      while(!digitalRead(18)) sg90.write(90);
      while(digitalRead(18)) delay(1000);
      sg90.write(0);
    }
  }
  // else {
  //   int value = digitalRead(sw420Pin);
  //   Serial.print(value);
  //   delay(100);
  // }
}