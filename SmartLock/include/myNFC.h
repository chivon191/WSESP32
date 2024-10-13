#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Wire.h>

class myNFC {
    public:
        String cardID, readID = "None"; 
        myNFC();
        void readNFC();
        int checkNFC();
    private:
        PN532_I2C pn532_i2c;
        NfcAdapter nfc;
}