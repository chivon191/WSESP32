#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <AS608.h>

const char* ssid = "FPT Telecom";
const char* password = "vonvaquoc";
const int ledPin = 16;

const char* serverName = "192.168.1.5"; 
const int serverPort = 8080; 

WebSocketsClient webSocket;  

FingerprintSensor vantay;

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

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
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
  if (Serial.available()) {
    char option = Serial.read();
    switch (option) {
      case '1':
        vantay.addFingerprint();
        break;
      case '2':
        vantay.deleteFingerprint();
        break;
      default:
        Serial.println("Lựa chọn không hợp lệ.");
        break;
    }
    delay(200);
  }  
}
