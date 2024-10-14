const int vibrationPin = 14;  // Chân GPIO kết nối cảm biến rung
const int ledPin = 2;         // LED để báo hiệu

void setup() {
  pinMode(vibrationPin, INPUT_PULLUP);  // Kích hoạt điện trở kéo lên nội bộ
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);                 // Khởi động Serial Monitor
}

void loop() {
  int vibrationState = digitalRead(vibrationPin); // Đọc tín hiệu từ cảm biến
  
  if (vibrationState == LOW) {  // Khi có rung động
    Serial.println("Rung động phát hiện!");
    digitalWrite(ledPin, HIGH); // Bật LED
  } else {
    Serial.println("Không có rung động");
    digitalWrite(ledPin, LOW); // Tắt LED
  }
  
  delay(500);  // Đợi 500 ms rồi kiểm tra lại
}
