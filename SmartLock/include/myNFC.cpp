#include "myNFC.h"

myNFC::myNFC() : pn532_i2c(Wire), nfc(pn532_i2c) 
{
    nfc.begin();
}

void myNFC::readNFC() {
    if(nfc.tagPresent())
    {
        NfcTag tag = nfc.read();
        readID = tag.getUidString();
        Serial.println("Tag ID");
        Serial.println(readID);
    }
    delay(1000);
}

int myNFC::checkNFC() {
    readNFC();
    if (readID == cardID or readID == tagID1) {
        return 1;
    }
    return 0;
}