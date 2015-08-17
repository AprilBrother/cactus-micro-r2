#include <avr/pgmspace.h>

//period between posts, set at 60 seconds
#define DELAY_PERIOD 10000

// Important!! We use pin 13 for enable Serial1  
#define WIFI_ENABLE_PIN 13

// TODO
#define SSID         "YOUR-WIFI-SSID"
#define PASS         "YOUR-WIFI-PASSWORD"

#define HOST         "192.168.60.225"
#define HOST_NAME    "localhost"

#define DEBUG         1

#define REQ_PREFIX    "POST /event.php\r\n" \
    "Host: " HOST_NAME "\r\n" \
    "Accept: *" "/" "*\r\n" \
    "Content-Length: " 

#define REQ_SUFFIX    "\r\n" \
    "Content-Type: application/x-www-form-urlencoded\r\n" \
    "Connection: close\r\n\r\n" 

#define printDebug(x)       Serial.print(F(x)); Serial1.print(F(x));
#define printlnDebug(x)     Serial.println(F(x)); Serial1.println(F(x));

#define printVar(x)         Serial.print(x); Serial1.print(x);
#define printlnVar(x)       Serial.println(x); Serial1.println(x);


long nextTime;
String prevString;

void clearRx() {
    while(Serial1.available() > 0) {
        Serial1.read();
    }
}

String readString(
    long timeout, 
    String target1, 
    String target2 = NULL, 
    String target3 = NULL,
    String target4 = NULL
) {
    String data;
    char a;
    unsigned long start = millis();

    while (millis() - start < timeout) {
        while(Serial1.available() > 0) {
            a = Serial1.read();
            if(a == '\0') {
                continue;
            }
            data += a;
            Serial.write(a);
        }

        if (data.indexOf(target1) != -1) {
            break;
        } 

        if (target2 && (data.indexOf(target2) != -1)) {
            break;
        }

        if (target3 && (data.indexOf(target3) != -1)) {
            break;
        }

        if (target4 && (data.indexOf(target4) != -1)) {
            break;
        }
    }

    return data;
}

bool connectWifi() {     
    bool connected = false;
    long timeout = 10000;
    clearRx();
    printDebug("AT+CWJAP=\"");
    printDebug(SSID);
    printDebug("\",\"");
    printDebug(PASS);
    printlnDebug("\"");
    
    String ret = readString(timeout, "OK", "FAIL");
    if (ret.indexOf("OK") != -1) {
        return true;
    }
    return false;
}

void setup() {

    Serial1.begin(9600);   //connection to ESP8266
    Serial.begin(9600);     //serial debug

    // For atmega32u4, Please set DEBUG = 0 if it not connected to USB
    if(DEBUG) {
        while(!Serial);
    }

    pinMode(WIFI_ENABLE_PIN, OUTPUT);
    digitalWrite(WIFI_ENABLE_PIN, HIGH);

    delay(1000);
    //set mode needed for new boards
    Serial1.println(F("AT+RST"));
    delay(3000);//delay after mode change       
    Serial1.println(F("AT+CWMODE=1"));
    delay(300);
    Serial1.println(F("AT+RST"));
    delay(3000);
    // Sinlge connection
    Serial1.println(F("AT+CIPMUX=0"));
    delay(500);

    if (connectWifi()) {
        Serial.print("Connected AP success\r\n");
    } else {
        Serial.print("Connect AP failure\r\n");
    }

    printlnDebug("AT+CIFSR");
    String ret = readString(10000, "STAIP"); 
    
    if (ret.indexOf("STAIP") != -1) {
        Serial.println(ret); 
    } else {
        Serial.println(F("Not got IP"));
    }

    nextTime = millis(); //reset the timer
}

void loop() {
    //wait for timer to expire
    if(nextTime < millis()){
        Serial.print("timer reset: ");
        Serial.println(nextTime);

        httpPost("test=123");

        //reset timer
        nextTime = millis() + DELAY_PERIOD;       
    }

}

//web request needs to be sent without the http for now, https still needs some working

void httpPost(String data) {    
    String ret;
    long timeout = 2000;
    Serial.println(F("Sending data..."));

    delay(1000);
    clearRx();
    printDebug("AT+CIPSTART=\"TCP\",\"");
    printDebug(HOST);
    printlnDebug("\",8080");
    
    ret = readString(timeout, "OK", "ALREAY CONNECT", "ERROR", "busy"); 

    //test for a start error
    if ((ret.indexOf("OK") != -1) || (ret.indexOf("ALREAY CONNECT") != -1)) {
        Serial.println(F("Connected"));
    } else {
        Serial.println(F("Not connected"));
        clearRx();
        printlnDebug("AT+CIPCLOSE");
        return;
    }   

    //send to ESP8266
    clearRx();
    delay(1000);
    printDebug("AT+CIPSEND="); 
    printlnVar( 
        String(REQ_PREFIX).length() + 
        String(data.length()).length() +
        data.length() +
        String(REQ_SUFFIX).length() 
    );

    ret = readString(timeout, String(">"), "link is not"); 
    Serial.print(F("Done:"));
    Serial.println(ret);
    if (ret.indexOf(">") != -1) {
        // Send data
        Serial.println(F("ready to send:"));
        Serial1.print(F(REQ_PREFIX));
        Serial1.print(String(data.length()));
        Serial1.print(F(REQ_SUFFIX));
        Serial1.print(data);

        ret = readString(timeout, "OK");
        //clearRx();
        printlnDebug("AT+CIPCLOSE");
    } else {
        printlnDebug("AT+CIPCLOSE");
        Serial.println(F("Connect timeout"));
        delay(1000);
    }
    
    clearRx();

}
