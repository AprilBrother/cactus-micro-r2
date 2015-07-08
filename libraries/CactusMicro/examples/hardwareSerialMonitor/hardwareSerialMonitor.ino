String tmp; 

#define BAUD_RATE 9600

void setup() {
  Serial.begin(BAUD_RATE);
  Serial1.begin(BAUD_RATE); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Hello Cactus Micro!");
  
  // Enable esp8266 
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);  
};

void loop() {

  while (Serial1.available() > 0)  {
    tmp += char(Serial1.read());
    delay(2);
  }

  if(tmp.length() > 0) {
    Serial.println(tmp);
    tmp = "";
  }

  if (Serial.available()) {
    Serial1.write(Serial.read());
  }
}
