#include "MyAS608.h"
#include "MyKeypad.h"
// #include "MyBuzzer.h"
#include "MyNFC.h"
#include <ESP32Servo.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <WiFi.h>

#define servoPin 2
#define sw420Pin 15
#define ledPin 13

LiquidCrystal_I2C  lcd(0x27, 16, 2);
MyKeypad keypad(lcd);
WebSocketsClient webSocket;  
// MyAS608 vantay(16, 17);
MyBuzzer buzzer(15);
MyNFC nfc;
Servo sg90;

int door_stat;
int alivePass = 0;
int firstClassSecurity = 0;
String correct_pass = "";

const char* ssid = "FPT Telecom";
const char* password = "vonvaquoc";


const char* serverName = "192.168.1.5"; 
const int serverPort = 8080; 

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
        while(!digitalRead(18)) sg90.write(90);
        Serial.println("Đã đóng cửa");
        while(digitalRead(18)) delay(1000);
        sg90.write(0);
        webSocket.sendTXT("closedoor");
      }
      break;
  }
}

bool check_layer1() {
  if (nfc.checkNFC()) {
    return true;
  }
  return false;
}

bool check_layer2() {
  Serial.println("Enter your password........");
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  if (keypad.check_password())
  {
    lcd.clear();
    lcd.setCursor(5,0); //-----Unlock------
    lcd.print("Unlock");
    lcd.setCursor(4,1); //-----Success-----
    lcd.print("Successs");
    buzzer.success();
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
    buzzer.error();
    Serial.println("\n------------------------------------------------");
    Serial.println("Unlock failed...........");
    Serial.println("------------------------------------------------");
    lcd.clear();
    return false;
  }
}


void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  delay(1);
  lcd.clear();
  lcd.setCursor(0,0);

  lcd.print("   1st Class");
  lcd.setCursor(0,1);
  lcd.print("    Security");
  pinMode(18, INPUT_PULLUP);
  pinMode(sw420Pin, INPUT);
  pinMode(ledPin, OUTPUT);
  //pinMode(vibrationPin, INPUT_PULLUP);
  sg90.attach(servoPin);
  sg90.write(3);
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
  else {
    int value = digitalRead(sw420Pin);
    if(value == 1) {
    buzzer.error();
    }
  }
}