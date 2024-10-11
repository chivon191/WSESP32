#include "myBuzzer.h"

myBuzzer::myBuzzer(const int pin) : buzzerPin(pin)
{
    pinMode(buzzerPin, OUTPUT);  
    ledcAttachPin(buzzerPin, 0); 
}

void myBuzzer::success() {
    ledcWriteTone(0, 2700);
    delay(100);
    ledcWriteTone(0,0);              
    delay(125);                         
    ledcWriteTone(0, 2700);    
    delay(100);        
}

void myBuzzer::error() {
    ledcWriteTone(0, 1000);
    delay(200);
    ledcWriteTone(0, 0);
}