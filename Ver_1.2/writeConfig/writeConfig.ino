#include <DHT.h>
#include <FS.h>
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

//Define MCU Digital Pins
#define D0 16
#define D1 5 // I2C Bus SCL (clock)
#define D2 4 // I2C Bus SDA (data)
#define D3 0
#define D4 2 // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO 
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3 // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)

//DHT22
#define DHTTYPE DHT22
DHT dht(D4, DHTTYPE);

//PMS5003
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];
int PM01Value = 0;          
int PM25Value = 0;
int PM10Value = 0;

/* Set these to your desired credentials. */
const char* nodeID = "FAirBox_0010";
const char* defaultPWD = "12345678";

//StaticJsonBuffer<100> jsonConfigBuffer;
//JsonObject& wifiConfig = jsonConfigBuffer.createObject();


const char* host = "118.70.72.15";
const int httpPort = 5902;

const int delayTime = 60; //seconds
ESP8266WebServer server(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPIFFS.begin();
  Serial.println("Start00");
  StaticJsonBuffer<100> jsonConfigBuffer;
  JsonObject& wifiConfig = jsonConfigBuffer.createObject();
  wifiConfig["ssid"] = "FIMO_TRUONGSA";
  wifiConfig["password"] = "fimo!123";
  writeConfig(wifiConfig);
}

void loop() {
  // put your main code here, to run repeatedly:

}
void writeConfig(JsonObject& wifiConfig){
    // open file for writing
  File f = SPIFFS.open("/config.json", "w");
  if (!f) {
      Serial.println("file open failed");
  }
  String wifiConfigString;
  wifiConfig.printTo(wifiConfigString);
  // write ssid and pwd to file
  f.println(wifiConfigString);
  f.close();
  
  Serial.println("Wroted");
}
