#ifndef MYAS608_H
#define MYAS608_H

#include <HardwareSerial.h>
#include <Adafruit_Fingerprint.h>

class MyAS608 {
  public:
    MyAS608(int const RXD2, int const TXD2);
    void addFingerprint();
    void deleteFingerprint(uint8_t id);
    bool checkFingerprint();
    uint8_t getFingerprintEnroll(uint8_t id);

  private:
    HardwareSerial mySerial;
    Adafruit_Fingerprint finger;
};

#endif // MYAS608_H
