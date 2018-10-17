#include <FS.h>
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#define sensorID 5

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

/* Set these to your desired credentials. */
const char* nodeID = "CO_5";
const char* defaultPWD = "12345678";

const char* host = "118.70.72.15";
const int httpPort = 2224;

const int delayTime = 4; //seconds
ESP8266WebServer server(80);

void setup() {
  Serial.println("Start");
  Serial.begin(9600);
  SPIFFS.begin();
  Serial.println("Init done");
  //Set LED pins as ouput
  //pinMode(LED_BUILTIN, OUTPUT); //POWER
  pinMode(D4, OUTPUT); //WIFI

  //turnON(LED_BUILTIN);
  
  start();
}

void loop() {
  if(WiFi.status() == WL_CONNECTION_LOST){
    turnOFF(D4);
    startAP();
  } else {
    if(postData()){
      turnON(D4);
    } else {
      turnOFF(D4);
    }
    delay(delayTime*1000);
  }
  server.handleClient();
}

void handleRoot() {
  server.send(200, "text/html", "ON");
}

void handleInfo() {
  server.send(200, "application/json", "{\"id\": \""+String(sensorID)+"\"}");
}

void handleScan() {
  server.send(200, "application/json", scanWifi());
}

void handleData() {
  server.send(200, "application/json", String(readData()));
}

void handleConfig() {
  StaticJsonBuffer<100> jsonConfigBuffer;
  JsonObject& wifiConfig = jsonConfigBuffer.createObject();
  wifiConfig["ssid"] = server.arg("ssid");
  wifiConfig["password"] = server.arg("pwd");
  writeConfig(wifiConfig);
  server.send(200, "text/html", "1");
  start();
}

void start(){
  connectWifi(readConfig());
  if(WiFi.status() == WL_CONNECTED){
    //Serial.println(WiFi.localIP());
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/info", handleInfo);
    server.on("/scan", handleScan);
    server.on("/config", handleConfig);
    server.begin();
  } else {
    startAP();
  }
}
void writeConfig(JsonObject& wifiConfig){
    // open file for writing
  File f = SPIFFS.open("/config.json", "w");
  if (!f) {
      //Serial.println("file open failed");
  }
  String wifiConfigString;
  wifiConfig.printTo(wifiConfigString);
  // write ssid and pwd to file
  f.println(wifiConfigString);
  f.close();
  
  //Serial.println("Wroted");
}

JsonObject& readConfig(){
  StaticJsonBuffer<100> jsonConfigBuffer;
  String stringConfig;
  // open file for reading
  File f = SPIFFS.open("/config.json", "r");
  if (!f) {
      //Serial.println("file open failed");
  }
  // read wifi SSID config
  while(f.available()){
    stringConfig += f.readStringUntil('\n');
  }
  char charsConfig[stringConfig.length()];
  stringConfig.toCharArray(charsConfig, stringConfig.length());
  return jsonConfigBuffer.parseObject(charsConfig);
}

void connectWifi(JsonObject& wifiConfig){
  WiFi.mode(WIFI_STA); //WiFi.mode(m): set mode to WIFI_AP, WIFI_STA, or WIFI_AP_STA
  delay(1000);
  const char* ssid = wifiConfig["ssid"];
  const char* password = wifiConfig["password"];
  WiFi.begin(ssid, password);
  int i;
  for(i=0; i < 20 ; i++){
    delay(1000);
    if(WiFi.status() == WL_CONNECTED)
      break;
  }
}

void startAP(){
  WiFi.disconnect();
  WiFi.mode(WIFI_AP); //WiFi.mode(m): set mode to WIFI_AP, WIFI_STA, or WIFI_AP_STA
  delay(1000);
  WiFi.softAP(nodeID, defaultPWD);
  //IPAddress myIP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/info", handleInfo);
  server.on("/scan", handleScan);
  server.on("/config", handleConfig);
  server.begin();
}

String scanWifi(){
  StaticJsonBuffer<2000> jsonScanBuffer;
  int n = WiFi.scanNetworks();
  JsonObject& list = jsonScanBuffer.createObject();
  JsonArray& listSSID = list.createNestedArray("ssid");
  if (n > 0)
  {
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      JsonObject& ssid = listSSID.createNestedObject();
      ssid["id"] = i;
      ssid["ssid"] = WiFi.SSID(i);
      ssid["rssi"] = WiFi.RSSI(i);
      ssid["isOpen"] = (WiFi.encryptionType(i) == ENC_TYPE_NONE)? true : false;
    }
  }
  String result;
  list.printTo(result);
  return result;
}

void turnON(int led){
  digitalWrite(led, HIGH);
}

void turnOFF(int led){
  digitalWrite(led, LOW);
}

//Read data from MQ7-sensor
int readData(){
  return analogRead(A0);
}

boolean postData(){
  int raw = readData();
  String url = "/data?id="+String(sensorID)+"&raw="+String(raw);
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    //Serial.println("connection failed");
    return false;
  }
  
  // This will send the request to the server
  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("Connection: close");
  client.println("");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 10000) {
      //Serial.println(">>> Client Timeout !");
      client.stop();
      return false;
    }
  }

  return true;
}

void blinkLED(int delayTime, int interval){
  int i = 0;
  for(i = 0; i < interval; i++){
    turnON(D4);
    delay(delayTime);
    turnOFF(D4);
    delay(delayTime);
  }
}
