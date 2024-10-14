#include "MyAS608.h"

MyAS608::MyAS608(int const RXD2, int const TXD2) : mySerial(2), finger(&mySerial) {
  // Khởi động Serial cho cảm biến vân tay
  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("\n\nAS608 Fingerprint sensor initialized.");

  // Khởi động cảm biến vân tay
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Cảm biến vân tay đã kết nối thành công!");
  } else {
    Serial.println("Không tìm thấy cảm biến vân tay :(");
    while (1) {
      delay(1); // Dừng chương trình nếu không tìm thấy cảm biến
    }
  }
}

void MyAS608::addFingerprint() {
  uint8_t id = 1; // Thay đổi ID khi cần thêm vân tay mới
  Serial.println("Bắt đầu thêm vân tay.");
  if (getFingerprintEnroll(id)) {
    Serial.println("Đăng ký vân tay thành công");
  } else {
    Serial.println("Đăng ký vân tay thất bại");
  }
}

uint8_t MyAS608::getFingerprintEnroll(uint8_t id) {
  Serial.print("Bắt đầu đăng ký vân tay cho ID: ");
  Serial.println(id);
  
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Ảnh vân tay đã được chụp.");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println("Không có ngón tay trên cảm biến.");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Lỗi nhận gói tin.");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Không thể chụp ảnh vân tay.");
        return p;
      default:
        Serial.println("Lỗi không xác định.");
        return p;
    }
  }

  // Tiếp tục xử lý và lưu ảnh vân tay
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Không thể chuyển ảnh thành mẫu vân tay.");
    return p;
  }

  // Đăng ký vân tay với ID
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Tạo mẫu vân tay thành công.");
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
      Serial.println("Lưu mẫu vân tay thành công.");
      return true;
    } else {
      Serial.println("Lưu mẫu vân tay thất bại.");
    }
  } else {
    Serial.println("Tạo mẫu vân tay thất bại.");
  }
  
  return false;
}

void MyAS608::deleteFingerprint(uint8_t id) {
  Serial.print("Xóa vân tay với ID: ");
  Serial.println(id);
  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    Serial.println("Xóa vân tay thành công.");
  } else {
    Serial.println("Xóa vân tay thất bại.");
  }
}

bool MyAS608::checkFingerprint() {
  Serial.println("Kiểm tra vân tay...");
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    Serial.println("Ảnh vân tay đã được chụp.");
    if (finger.image2Tz() == FINGERPRINT_OK) {
      if (finger.fingerSearch() == FINGERPRINT_OK) {
        Serial.print("Tìm thấy vân tay với ID: ");
        Serial.println(finger.fingerID);
        return true;
      } else {
        Serial.println("Không tìm thấy vân tay.");
      }
    } else {
      Serial.println("Không thể chuyển ảnh thành mẫu vân tay.");
    }
  } else {
    Serial.println("Không có ngón tay trên cảm biến.");
  }
  return false;
}
