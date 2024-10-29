#include <Adafruit_Fingerprint.h>
#define LED 2
#define RXD2 16   // Chân RX của ESP32 (kết nối với TX của AS608)
#define TXD2 17   // Chân TX của ESP32 (kết nối với RX của AS608)
#define tchFinger 19

HardwareSerial mySerial(2);  // Serial2 sử dụng TXD2 và RXD2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(tchFinger, INPUT);
  digitalWrite(LED, LOW);
  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);

  // Khởi động cảm biến vân tay
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Cảm biến vân tay đã kết nối thành công!");
  } else {
    Serial.println("Không tìm thấy cảm biến vân tay!");
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

bool deleteFingerprint() {
  Serial.println("Vui lòng scan vân tay muốn xóa...");
  for (int i=0; i<3; i++) {
    while(!digitalRead(tchFinger)) {
      Serial.print("Đặt tay lên cảm biến....");
      delay(500);
    }
    if (finger.getImage() == FINGERPRINT_OK) {
      if (finger.image2Tz() == FINGERPRINT_OK) {
        if (finger.fingerFastSearch() == FINGERPRINT_OK) {
          id = finger.fingerID;
          if (finger.deleteModel(id) == FINGERPRINT_OK) {
            Serial.println("Đã xóa vân tay thành công!");
            return true;
          } else {
            Serial.println("Xóa vân tay thất bại");
          }
        } else {
          Serial.println("Không tìm thấy vân tay cần xóa");
        }
      } else {
        Serial.println("Không thể chuyển đổi hình ảnh vân tay");
      }
    } else {
      Serial.println("Không tìm thấy vân tay.");
    }
  }
  return false;
}

bool checkFingerprint() {
  Serial.println("Đặt ngón tay lên cảm biến để kiểm tra...");
  for (int i=0; i<3; i++) {
    while(!digitalRead(tchFinger)) {
      Serial.print("Đặt tay lên cảm biến....");
      delay(500);
    }
    if (finger.getImage() == FINGERPRINT_OK) {
      if (finger.image2Tz() == FINGERPRINT_OK) {
        if (finger.fingerFastSearch() == FINGERPRINT_OK) {
          Serial.print("Xác thực vân tay thành công với ID ");
          Serial.println(finger.fingerID);
          return true;
        } else {
          Serial.println("Không tìm thấy vân tay.");
        }
      } else {
        Serial.println("Không thể chuyển đổi hình ảnh vân tay");
      }
    } else {
      Serial.println("Không tìm thấy vân tay.");
    }
  }
  return false;
}

int getNextAvailableID() {
  for (int i = 1; i < 127; i++)
    if (finger.loadModel(i) != FINGERPRINT_OK) 
      return i;
  return 0;
}

bool getFingerprintEnroll() {
  int p = -1;
  while(!digitalRead(tchFinger)) {
      Serial.print("Đặt tay lên cảm biến....");
      delay(500);
  }
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      delay(500); // Đợi một chút trước khi kiểm tra lại
    } else if (p == FINGERPRINT_IMAGEFAIL) {
      Serial.println("Lỗi chụp hình ảnh");
      return p;
    } else if (p != FINGERPRINT_OK) {
      Serial.println("Lỗi không xác định");
      return p;
    }
  }

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Lỗi chuyển đổi hình ảnh");
    return false;
  }
  Serial.println("Lấy dấu vân tay lần 1 thành công. Hãy tháo tay.");
  delay(2000);

  while(!digitalRead(tchFinger)) {
      Serial.print("Đặt tay lên cảm biến để lấy dấu lần 2...");
      delay(500);
  }
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      delay(500); // Đợi một chút trước khi kiểm tra lại
    } else if (p == FINGERPRINT_IMAGEFAIL) {
      Serial.println("Lỗi chụp hình ảnh");
      return p;
    } else if (p != FINGERPRINT_OK) {
      Serial.println("Lỗi không xác định");
      return p;
    }
  }

  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Lỗi chuyển đổi hình ảnh lần 2");
    return false;
  }

  if (finger.createModel() == FINGERPRINT_OK) {
    if(finger.fingerFastSearch() == FINGERPRINT_OK) {
      Serial.println("Trùng với vân tay đã đăng ký!");
    }
    else {
      id = getNextAvailableID();
      finger.storeModel(id);
      Serial.print("Đăng ký vân tay thành công với ID ");
      Serial.println(id);
      return true;
    }
  }
  else {
    Serial.println("Đăng ký vân tay thất bại.");
    return false;
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