/*
* Edited by Mert Gülsoy
*   *While the chip is running at 8MHZ the clock speed is enough to transfer even single char received
*/

#define BAUD_RATE 9600
#define ESP Serial1

void setup() {
  Serial.begin(BAUD_RATE);
  ESP.begin(BAUD_RATE); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  for(int i = 0; i<5; i++) {
    Serial.print(i,DEC);
    Serial.println(F(" Hello Cactus Micro!"));
    delay(1010);
  }
  
  // Enable esp8266 
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);  
};

void loop() {

  if(ESP.available() > 0)
    Serial.write(ESP.read());


  if (Serial.available()) 
    ESP.write(Serial.read());
}
