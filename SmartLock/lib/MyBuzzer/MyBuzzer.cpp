#include "MyBuzzer.h"
#include <Arduino.h>

MyBuzzer::MyBuzzer(const int pin) : buzzerPin(pin)
{
    pinMode(buzzerPin, OUTPUT);  
    ledcAttachPin(buzzerPin, 0); 
}

void MyBuzzer::success() {
  tone(buzzerPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
  delay(125); // Đợi 125 ms
  tone(buzzerPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
  noTone(buzzerPin);
}

// Hàm báo lỗi
void MyBuzzer::error() {
  tone(buzzerPin, 200, 1000); 
}