#include <MyAS608.h>
#include <MyBuzzer.h>
#include <MyNFC.h>
#include <ESP32Servo.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "FPT Telecom";
const char* password = "vonvaquoc";
const char* serverName = "192.168.1.5"; 
const int serverPort = 8080; 

#define servoPin 2
#define sw420Pin 15

LiquidCrystal_I2C  lcd(0x27, 16, 2);
WebSocketsClient webSocket;   
MyAS608 vantay(16, 17);
MyBuzzer buzzer(0);
MyNFC nfc;
Servo sg90;

int door_stat;
int alivePass = 0;
int firstClassSecurity = 0;

const byte rows = 4;
const byte columns = 4;
String correct_pass = "1234";

char keys[rows][columns] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};

byte rowPins[rows] = {13, 4, 14, 27};
byte columnPins[columns] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
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
        Serial.println("Đã mở cửa");
        // while(!digitalRead(18)) sg90.write(90);
        Serial.println("Đã đóng cửa");
        // while(digitalRead(18)) delay(1000);
        // sg90.write(0);
        webSocket.sendTXT("closedoor");
      }
      break;
  }
}
char read_character() {
    return keypad.getKey(); // Trả về '\0' nếu không có key nào
}

int check_password() {
  String pass = "";
  char key = '\0';
  int size = 6; // Kích thước mật khẩu
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("ENTER PASSWORD");
  lcd.setCursor(5, 1);
  for (int count = 0; count < 3; count++) {
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

int change_password() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Change password");
  delay(1000);
  Serial.println("Enter your password.....");
  int check = check_password();
  if (check == 1) {
    String newPass1 = "";
    String newPass2 = "";
    int times_enter = 2;
    while (times_enter) {
      String pass = "";
      int size = 6;
      if (times_enter == 2) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter new pass");
        Serial.println("\nEnter your new password.......");
      }
      else {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Re-enter pass");
        Serial.println("\nRe-enter your new password......");
      }
      lcd.setCursor(5,1);
      while (size) {
        char key = read_character();
        delay(100);
        if (key >= '1' && key <= '9') {
          Serial.print(key);
          lcd.print(key);
          pass += String(key);
          key = '\0';
          size--;
        }
        else if (key == 'D') {
          size = 6;
          pass = "";
          lcd.setCursor(5,1);
          lcd.print("      ");
          lcd.setCursor(5,1);
          Serial.println("\nReset enter password...");
        }
        else if (key == 'A') {
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
    if (newPass1 == newPass2) {
      correct_pass = newPass1;
      return 1;
    }
  }
  return 0;
}

int menu() {
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
  Serial.println("2. Add/Delete card.");
  Serial.println("3. Add/Delete fingerprint.");
  Serial.println("------------------------------------------------");
  while (time) {
    key = read_character();
    delay(100);
    if ((key >= '1' && key <= '3') || key == 'C') break;
    time--;
  }
  if (time) {
    if (key == '1') {
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
    else if (key == '2') {
      lcd.clear();
      Serial.println("1. Add new card.");
      Serial.println("2. Delete card.");
    }
    else if (key == '3') {
      Serial.println("1. Add new fingerprint.");
      Serial.println("2. Delete fingerprint.");

      if (key == '1') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Đặt vân tay lên cảm biến");
        if(vantay.addFingerprint()) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Add finger successfull");
        }
        else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Add finger error");
        }
      }

      if (key == '2') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Đặt vân tay cần xóa lên cảm biến");
        if(vantay.deleteFingerprint()) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Delete finger successfull");
        }
        else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Delete finger error");
        }
      } 
    }
  }
  return 0;
}

bool check_layer1() {
  char key = read_character();
  if (key == '1')
  {
    while (true) {
      if(vantay.checkFingerprint()) {
        buzzer.success();
        return true;
      }
      else {
        buzzer.error();
        return false;
      }
    }
  }
  if (key == '2')
  {
    
    if (nfc.checkNFC()) {
      buzzer.success();
      return true;
    }
    else {
      buzzer.error();
      return false;
    }
  }
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
  lcd.clear();
  lcd.setCursor(0,0);

  lcd.print("   1st Class");
  lcd.setCursor(0,1);
  lcd.print("    Security");
  pinMode(sw420Pin, INPUT);
  pinMode(18, INPUT_PULLUP);

  sg90.attach(servoPin);
  sg90.write(3);
  // nfc.begin();
  // mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);

  // Serial.println("\n\nAS608 Fingerprint sensor with add/delete/check");

  // // Khởi động cảm biến vân tay
  // finger.begin(57600);
  // if (finger.verifyPassword()) {
  //   Serial.println("Cảm biến vân tay đã kết nối thành công!");
  // } else {
  //   Serial.println("Không tìm thấy cảm biến vân tay :(");
  //   while (1) { delay(1); }
  // }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  Serial.println("Đã kết nối WiFi.");

  webSocket.begin(serverName, serverPort, "/ws/?type=esp32"); 
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket đang chờ kết nối...");
  delay(2000);
}

void loop() {
  webSocket.loop();
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