int door_stat;

void setup() 
{
  Serial.begin(9600);
  pinMode(14, INPUT_PULLUP);
  pinMode(2, OUTPUT);
  
}

void loop() 
{
  door_stat = digitalRead(14);
  digitalWrite(2,door_stat);
  Serial.println(door_stat);
  delay(100);
}
