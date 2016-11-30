/**
   \file
         Temperature logger example
   \author
         Yan <yancan@aprbrother.com>
*/

#include <espduino.h>
#include <rest.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_ENABLE_ESP 13

//pin for the DS18B20 temperature module
#define TEMPERATURE_PIN 14

#define SSID  "YOUR-SSID"
#define PASS  "YOUR-WIFI-PASS"

#define PUBLIC_KEY "YOUR-PUBLIC-KEY-SPARKFUN"
#define PRIVATE_KEY "YOUR-PRAVATE-KEY-SPARKFUN"

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

// use http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
// to find the address of your DS18B20, it will be different then this
DeviceAddress thermometer = { 0x28, 0x60, 0xB7, 0xCC, 0x04, 0x00, 0x00, 0x76 };

ESP esp(&Serial1, &Serial, PIN_ENABLE_ESP);

REST rest(&esp);

boolean wifiConnected = false;

void wifiCb(void* response)
{
  uint32_t status;
  RESPONSE res(response);

  if (res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if (status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");

      wifiConnected = true;
    } else {
      wifiConnected = false;
    }

  }
}

void setup() {
  Serial1.begin(19200);
  Serial.begin(19200);

  sensors.begin();

  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while (!esp.ready());

  Serial.println("ARDUINO: setup rest client");
  if (!rest.begin("data.sparkfun.com")) {
    Serial.println("ARDUINO: failed to setup rest client");
    while (1);
  }

  /*setup wifi*/
  Serial.println("ARDUINO: setup wifi");
  esp.wifiCb.attach(&wifiCb);

  esp.wifiConnect(SSID, PASS);
  Serial.println("ARDUINO: system started");
}

void loop() {
  char response[266];
  esp.process();
  if (wifiConnected) {
    char buff[200];
    char temp[10];
    float temperature;

    //get temp
    sensors.requestTemperatures();
    temperature = sensors.getTempC(thermometer);
    Serial.print("temp: ");
    Serial.println(temperature);
    dtostrf(temperature, 1, 2, temp);

    //send temp
    sprintf(buff, "/input/%s?private_key=%s&temp=%s", PUBLIC_KEY, PRIVATE_KEY, temp);
    Serial.println(buff);

    rest.get((const char*)buff);
    Serial.println("ARDUINO: send get");

    if (rest.getResponse(response, 266) == HTTP_STATUS_OK) {
      Serial.println("ARDUINO: GET successful");
      Serial.println(response);
    }
    delay(30000);
  }
}
