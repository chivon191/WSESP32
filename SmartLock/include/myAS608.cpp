#include "myAS608.h"

myAS608::myAS608() :
{
  HardwareSerial mySerial(2);  
  finger = Adafruit_Fingerprint(&mySerial);
  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("\n\nAS608 Fingerprint sensor with add/delete/check");
  finger.begin(57600);
  if (finger.verifyPassword()) {
      Serial.println("Cảm biến vân tay đã kết nối thành công!");
  } else {
      Serial.println("Không tìm thấy cảm biến vân tay :(");
      while (1) { delay(1); }
  }
}

void myAS608::addFingerprint() {
  Serial.println("Nhập ID vân tay muốn lưu (1 - 127):");
  uint8_t id = readNumber();
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

void myAS608::deleteFingerprint() {
  Serial.println("Nhập ID vân tay muốn xóa (1 - 127):");
  uint8_t id = readNumber();
  if (id == 0) return;

  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    Serial.println("Đã xóa vân tay thành công!");
  } else {
    Serial.println("Xóa vân tay thất bại!");
  }
}

void myAS608::checkFingerprint() {
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

uint8_t myAS608::getFingerprintEnroll() {
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

uint8_t myAS608::readNumber() {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}