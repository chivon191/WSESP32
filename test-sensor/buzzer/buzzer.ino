// Chân GPIO kết nối với loa Piezo
const int piezoPin = 0;

void setup() {
  // Khởi tạo chân GPIO làm đầu ra
  pinMode(piezoPin, OUTPUT);
}

void loop() {
  // Gọi hàm báo thành công
  success();
  delay(1000); // Đợi 1 giây

  // Gọi hàm báo lỗi
  error();
  delay(1000); // Đợi 1 giây
}

// Hàm báo thành công
void success() {
  tone(piezoPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
  delay(125); // Đợi 125 ms
  tone(piezoPin, 2700, 100); // Tần số 2700 Hz, thời gian 100 ms
}

// Hàm báo lỗi
void error() {
  tone(piezoPin, 200, 1000); // Tần số 200 Hz, thời gian 1000 ms
}
