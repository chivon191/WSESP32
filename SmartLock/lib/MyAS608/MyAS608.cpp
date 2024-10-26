#include "MyAS608.h"

MyAS608::MyAS608(int const RXD2, int const TXD2) : mySerial(2), finger(&mySerial), id(0) {
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

bool MyAS608::addFingerprint() {
  id++;
  Serial.println("Bắt đầu thêm vân tay.");
  if (getFingerprintEnroll(id)) {
    Serial.println("Đăng ký vân tay thành công");
    return true;
  } else {
    Serial.println("Đăng ký vân tay thất bại");
    return false;
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

bool MyAS608::deleteFingerprint() {
  int attempts = 0;  // Biến đếm số lần thử quét vân tay
  uint8_t id = 0;    // ID của vân tay cần xóa

  // Yêu cầu người dùng quét vân tay
  while (attempts < 3) {  // Cho phép người dùng thử tối đa 3 lần
    Serial.println("Đặt ngón tay lên cảm biến để xóa...");
    uint8_t p = finger.getImage();

    if (p == FINGERPRINT_OK) {  // Người dùng đã đặt ngón tay lên cảm biến
      attempts++;  // Tính là 1 lần thử

      // Chuyển đổi vân tay thành dữ liệu để kiểm tra
      p = finger.image2Tz();
      if (p == FINGERPRINT_OK) {
        // Tìm kiếm vân tay trong cơ sở dữ liệu
        p = finger.fingerFastSearch();
        if (p == FINGERPRINT_OK) {
          id = finger.fingerID;  // Lưu ID của vân tay được tìm thấy
          Serial.print("Vân tay được phát hiện với ID #");
          Serial.println(id);
          break;  // Thoát khỏi vòng lặp nếu tìm thấy
        } else {
          Serial.println("Không tìm thấy vân tay!");
        }
      }
    } else {
      Serial.println("Lỗi khi đọc vân tay hoặc chưa đặt ngón tay lên cảm biến.");
    }
  }

  // Nếu đã tìm thấy vân tay và ID hợp lệ, tiến hành xóa
  if (id > 0 && attempts <= 3) {
    Serial.print("Xóa vân tay với ID: ");
    Serial.println(id);
    if (finger.deleteModel(id) == FINGERPRINT_OK) {
      Serial.println("Xóa vân tay thành công.");
      return true;
    } else {
      Serial.println("Xóa vân tay thất bại.");
      return false;
    }
  } else {
    Serial.println("Không thể xác định vân tay để xóa.");
    return false;
  }
}


bool MyAS608::checkFingerprint() {
  int attempts = 0;  // Biến đếm số lần thử quét vân tay

  while (attempts < 3) {  // Cho phép người dùng thử tối đa 3 lần
    Serial.println("Đặt ngón tay lên cảm biến để kiểm tra...");
    uint8_t p = finger.getImage();
    
    if (p == FINGERPRINT_OK) {  // Người dùng đã đặt ngón tay lên cảm biến
      attempts++;  // Tính là 1 lần thử
      Serial.print("Số lần thử hiện tại: ");
      Serial.println(attempts);

      p = finger.image2Tz();  // Tiếp tục xử lý vân tay
      if (p == FINGERPRINT_OK) {
        p = finger.fingerFastSearch();
        if (p == FINGERPRINT_OK) {
          Serial.print("Đã nhận diện thành công ID #");
          Serial.print(finger.fingerID);
          Serial.print(" với độ tin cậy: ");
          Serial.println(finger.confidence);
          return true;  // Nhận diện thành công, trả về true
        } else {
          Serial.println("Không tìm thấy vân tay!");
        }
      } 
    } else {
      Serial.println("Lỗi khi đọc vân tay hoặc chưa đặt ngón tay lên cảm biến.");
    }

    // Nếu người dùng không đặt ngón tay hoặc gặp lỗi thì sẽ không tăng attempts
  }

  Serial.println("Bạn đã vượt quá số lần thử vân tay cho phép.");
  return false;  // Nếu quá 3 lần mà vẫn không thành công, trả về false
}
