#include <Adafruit_Fingerprint.h>
#define LED 2
#define RXD2 16   // Chân RX của ESP32 (kết nối với TX của AS608)
#define TXD2 17   // Chân TX của ESP32 (kết nối với RX của AS608)

HardwareSerial mySerial(2);  // Serial2 sử dụng TXD2 và RXD2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
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
}

void loop() {
  // Kiểm tra vân tay liên tục
  checkFingerprint();

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
    delay(100); // tránh lặp quá nhanh
  }
}

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
        digitalWrite(LED, HIGH);
        delay(2000);
        digitalWrite(LED, LOW);
      } else {
        Serial.println("Không tìm thấy vân tay!");
      }
    } else {
      Serial.println("Không chuyển đổi được hình ảnh vân tay.");
    }
  } else if (p == FINGERPRINT_NOFINGER) {
    // Không có vân tay, có thể bỏ qua
  } else {
    Serial.println("Lỗi không xác định.");
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
}

uint8_t readNumber() {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}