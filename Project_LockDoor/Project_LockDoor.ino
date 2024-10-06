#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);

NfcAdapter nfc = NfcAdapter(pn532_i2c);
String cardId1 = "63 6B 6D 0B";
String tagId1= "E1 B2 99 02";
String tagId = "None";
byte nuidPICC[4];

// Chân GPIO kết nối với loa Piezo
const int piezoPin = 12;
const int vibrationPin = 14;  // Chân GPIO kết nối cảm biến rung

const char* ssid = "FPT Telecom";
const char* password = "vonvaquoc";

const int ledPin = 16;

const char* serverName = "192.168.1.5"; 
const int serverPort = 8080; 
const int door_stat;

WebSocketsClient webSocket;  

void setup() {
  Serial.begin(115200);

  pinMode(18, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  pinMode(vibrationPin, INPUT_PULLUP);

  digitalWrite(ledPin, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  Serial.println("Đã kết nối WiFi.");

  webSocket.begin(serverName, serverPort, "/ws/?type=esp32"); 
  webSocket.onEvent(webSocketEvent);

  Serial.println("WebSocket đang chờ kết nối...");
  
}

void loop() {
  webSocket.loop();

  door_stat = digitalRead(14);

  Serial.println(door_stat);

  if (vibrationState == LOW) // Khi có rung động
  {  
    Serial.println("Rung động phát hiện!");
  } 
  else 
  {
    Serial.println("Không có rung động");
  }

  readNFC();

  if(tagId==cardId1 or tagId==tagId1)
  {
    
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
}

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