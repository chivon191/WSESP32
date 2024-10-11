#include <Adafruit_Fingerprint.h>

class myAS608 {
    public:
        const int RXD2 = 16;
        const int TXD2 = 17;
        Adafruit_Fingerprint finger;
        myAS608();
        void addFingerprint();
        void deleteFingerprint();
        void checkFingerprint();
        uint8_t getFingerprintEnroll();
        uint8_t readNumber();
};



