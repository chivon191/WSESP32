#include "MyNFC.h"

MyNFC::MyNFC() : pn532_i2c(Wire), nfc(pn532_i2c) 
{
    nfc.begin();
}

void MyNFC::readNFC() {
    if(nfc.tagPresent())
    {
        NfcTag tag = nfc.read();
        readID = tag.getUidString();
        Serial.println("Tag ID");
        Serial.println(readID);
    }
    delay(1000);
}

int MyNFC::checkNFC() {
    readNFC();
    if (readID == cardID or readID == tagID) {
        return 1;
    }
    return 0;
}