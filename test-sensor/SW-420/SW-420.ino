// Khai báo chân của cảm biến rung
#define VIBRATION_SENSOR_PIN 12  // ADC1_0 tương ứng với chân GPIO 36 trên ESP32
#define ledPin 13

void setup() {
  // Khởi tạo cổng Serial để xuất dữ liệu
  Serial.begin(115200);

  // Cấu hình chân cảm biến rung làm đầu vào
  pinMode(VIBRATION_SENSOR_PIN, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Đọc giá trị từ cảm biến rung (giá trị ADC từ 0 đến 4095)
  int sensorValue = digitalRead(VIBRATION_SENSOR_PIN);
  Serial.print("Giá trị cảm biến rung: ");
  Serial.println(sensorValue);
  // In giá trị đọc được ra Serial Monitor
  if(sensorValue == 1) {
    digitalWrite(ledPin, HIGH);
    delay(1000);
  }
  digitalWrite(ledPin, LOW);
  delay(1);
}
