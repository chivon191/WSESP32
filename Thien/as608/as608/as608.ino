#include <Adafruit_Fingerprint.h>
#include <Keypad.h>

const byte rows = 4;
const byte columns = 4;

char keys[rows][columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};

byte rowPins[rows] = {13, 4, 14, 27};
byte columnPins[columns] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);
HardwareSerial mySerial(1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int buttonPin = 18;
int tchFinger = 19;

void setup()  
{
  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);
  pinMode(buttonPin, INPUT);
  pinMode(tchFinger, INPUT);
  while(!finger.verifyPassword());
  delay(2000);
  Serial.println("Beginning scan finger.....");
  Serial.println("--------------------------------");
  reset_memory();
}

void reset_memory()
{
  Serial.println("Press 1 to reset.");
  Serial.println("--------------------------------");
  char your_choice;
  int countTime = 100;
  while(countTime)
  {
    your_choice = read_character();
    if(your_choice == 'A' || your_choice == '1') break;
    delay(100);
    countTime--;
  }
  if(your_choice == '1')
  {
    for(int i=10;i<40;i++)
    {
      finger.deleteModel(i);
    }
    Serial.println("Reset success.");
    Serial.println("--------------------------------");
  }
}

void printInformation()
{
  finger.getTemplateCount();
  Serial.print("Sensor contains ");
  int quantity = finger.templateCount;
  Serial.print(finger.templateCount); 
  Serial.println(" templates");
  for(int i=1;i<=127 && quantity;i++)
  {
    if(finger.loadModel(i) == FINGERPRINT_OK)
    {
      quantity--;
      Serial.print(String(i) + " ");
    }
  }
  Serial.println();
  Serial.println("Waiting for valid finger...");
  Serial.println("--------------------------------");
}

char read_character() {
  char key = keypad.getKey();
  if (key) return key;
  return '\0';
}

int getFingerprintID() {
  while(!digitalRead(tchFinger));
  int countTime = 20;
  int p = -1;
  while(p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    delay(100);
    countTime--;
  }

  if (p != FINGERPRINT_OK)  return 444;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return 445;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return 404;
  
  return finger.fingerID;
}

int getNextAvailableID() {
  for (int i = 1; i < 127; i++)
    if (finger.loadModel(i) != FINGERPRINT_OK) 
      return i;
  return 0;
}

int add_finger()
{
  int position = -1;
  int fingers[3] = {10, 20, 30};
  for(int i=0;i<3;i++)
  {
    if(finger.loadModel(fingers[i]) != FINGERPRINT_OK)
    {
      position = fingers[i];
      break;
    }
  }
  if(position == -1) return 2;
  Serial.println("Position: " + String(position));
  Serial.println("--------------------------------");
  for(int i=0;i<10;i++)
  {
    Serial.println(i);
    while(true)
    {
      while(!digitalRead(tchFinger));
      int p = -1;
      while (p != FINGERPRINT_OK)
      {
        p = finger.getImage();
        delay(100);
      }

      p = finger.image2Tz();
      if(p == FINGERPRINT_OK)
      {
        p = finger.fingerFastSearch();
        if(p != FINGERPRINT_OK)
        {
          finger.storeModel(position);
          Serial.println("Add success.....");
          Serial.println("Take your hand off the sensor.....");
          Serial.println("--------------------------------");
          delay(2000);
          position++;
          break;
        }
      }
    }
  }
  return 1;
}

int enrollFingerprint() {
  int id = getNextAvailableID();
  Serial.print("Position empty: ");
  Serial.println(id);

  // Lần quét đầu tiên
  Serial.println("Place your finger on the sensor.....");
  Serial.println("--------------------------------");
  while(!digitalRead(tchFinger));
  while (finger.getImage() != FINGERPRINT_OK);
  finger.image2Tz(1);
  Serial.println("Success.....");
  delay(2000);
  
  // Lần quét thứ hai
  Serial.println("Again.....");
  Serial.println("--------------------------------");
  while(!digitalRead(tchFinger));
  while (finger.getImage() != FINGERPRINT_OK);
  finger.image2Tz(2);
  
  // Kiểm tra độ trùng khớp và lưu
  if (finger.createModel() == FINGERPRINT_OK) {
    if (finger.storeModel(id) == FINGERPRINT_OK) {
      Serial.println("Success!");
      return 1;
    }
  }
  Serial.println("Unsuccess!");
  return 0;
}

int deleteFingerprintID() {
  int id = getFingerprintID();
  if(id == 404 || id == 444)
  {
    Serial.println("Error " + String(id));
    return 0;
  }
  id = (floor) (id / 10);
  id *= 10;
  Serial.println("Id: " + String(id));
  for(int i=id;i<id+10;i++)
    finger.deleteModel(i);
  return 1;
}

void setting()
{
  Serial.println("Setting:");
  Serial.println("1. Add new fingerprint");
  Serial.println("2. Delete fingerprint");
  Serial.println("3. Reset database");
  Serial.println("--------------------------------");
  char your_choice;
  int countTime = 300;
  while(countTime)
  {
    your_choice = read_character();
    if(your_choice >= '1' && your_choice <= '3' || your_choice == 'A') break;
    countTime--;
    delay(100);
  }
  if(your_choice == 'A') return;
  int index;
  switch (your_choice)
  {
  case '1':
    Serial.println("Beginning add fingerprint");
    Serial.println("--------------------------------");
    delay(1000);
    add_finger();
    break;
  case '2':
    Serial.println("Beginning delete fingerprint");
    Serial.println("--------------------------------");
    delay(1000);
    if(deleteFingerprintID() == 1) Serial.println("Delete success.");
    else Serial.println("Unsuccess");
    Serial.println("--------------------------------");
    break;
  case '3':
    Serial.println("Not yet");
    break;
  default:
    Serial.println("Just press button 1-3 / 'A'");
    break;
  }
}

void loop()
{
  if(!digitalRead(buttonPin))
  {
    printInformation();
    Serial.println("BUTTON PRESS");
    Serial.println("--------------------------------");
    while(!digitalRead(buttonPin));
    Serial.println("BUTTON RELEASE");
    Serial.println("--------------------------------");
    Serial.println("Please, place your finger on the sensor.....");
    Serial.println("--------------------------------");
    while(!digitalRead(tchFinger));
    int statusFingerCurrent = getFingerprintID();
    if(statusFingerCurrent == 444) Serial.println("Error");
    else if(statusFingerCurrent == 404)
    {
      Serial.println("Fingerprints do not match");
      Serial.println("--------------------------------");
    }
    else
    {
      Serial.println("ALREADY EXISTS");
      Serial.println("--------------------------------");
    }
    Serial.println("Press B to setting");
    Serial.println("--------------------------------");
    char your_choice;
    int countTime = 300;
    while(countTime)
    {
      your_choice = read_character();
      if(your_choice == 'A' || your_choice == 'B') break;
      countTime--;
      delay(100);
    }
    if(your_choice == 'B') setting();
    Serial.println("Press button to continue.....");
    Serial.println("--------------------------------");
  }
  delay(100);
}