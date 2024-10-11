#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Wire.h>

class myNFC {
    public:
        String cardID, tagID1, tagID2, readID = ""; 
        myNFC();
        void readNFC();
    private:
        PN532_I2C pn532_i2c;
        NfcAdapter nfc;
}