#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Wire.h>

class MyNFC {
    public: 
        MyNFC();
        void readNFC();
        int checkNFC();
        String readID = "";
    private:
        PN532_I2C pn532_i2c;
        NfcAdapter nfc;
        String cardID = "63 6B 6D 0B";
        String tagID = "E1 B2 99 02";
};