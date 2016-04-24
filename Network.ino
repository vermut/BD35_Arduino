#include <ESP8266WiFi.h>
// #include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

extern "C" {
#include "user_interface.h"
}

const char * ssid = "Dungeon";
const char * pass = "Tallinas";

IPAddress ip = IPAddress(10, 30, highByte(ESP.getChipId()) % 254, lowByte(ESP.getChipId()) % 254);
IPAddress gw = IPAddress(10, 30, 0, 1);
IPAddress mask = IPAddress(255, 255, 0, 0);
IPAddress dns = IPAddress(8, 8, 8, 8);

void net_enable() {
  WiFi.mode(WIFI_STA);
  WiFi.scanNetworks();
  WiFi.begin(ssid, pass);
  WiFi.config(ip, gw, mask, dns);

  //  while (!net_is_connected()) {
  //    delay(50);
  //  }
}

void net_connect() {
  ETS_UART_INTR_DISABLE();
  wifi_station_connect();
  ETS_UART_INTR_ENABLE();

}

void net_disconnect() {
  ETS_UART_INTR_DISABLE();
  wifi_station_disconnect();
  ETS_UART_INTR_ENABLE();

}

void net_wait_connect() {
  Serial.println("Connecting Wifi...");
  while (!net_is_connected()) {
    delay(50);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

bool net_is_connected() {
  return (WiFi.isConnected());
}

void net_disable() {
  WiFi.mode(WIFI_OFF);
}
/*
void net_register(float medianDistance) {
  StaticJsonBuffer<200> json;
  char jsonBuffer[256];

  JsonObject& root = json.createObject();
  root["id"] = ESP.getChipId();
  root["startup_distance"] = medianDistance;
  root.printTo(jsonBuffer, sizeof(jsonBuffer));

  http.begin("http://10.30.0.247:5000/sensor_register");
  http.POST(jsonBuffer);
  Serial.println(http.getString());
  http.end();
}*/

void net_report(int lastVal1, int lastVal2) {
  Serial.print("Ping: ");
  Serial.print(lastVal1);
  Serial.print(", ");
  Serial.print(lastVal2);
  Serial.println("cm");

  StaticJsonBuffer<200> json;
  char jsonBuffer[256];

  JsonObject& root = json.createObject();
  root["id"] = ESP.getChipId();
  root["distance"] = lastVal1;
  root["distance2"] = lastVal2;
  root["voltage"] = ESP.getVcc();
  root.printTo(jsonBuffer, sizeof(jsonBuffer));
  // root.printTo(Serial);

  http.begin("http://10.30.0.247:5000/sensor_record");
  http.POST(jsonBuffer);
  http.end(); 

  /*
  // http.begin("http://search-dungeon-qcc7an54euyyq4vnstm2j673ve.us-east-1.es.amazonaws.com/dungeon/sensor_record");
  http.begin("http://52.72.230.200/dungeon/sensor_record");
  http.POST(jsonBuffer);
  // Serial.println(http.getString());
  http.end();
  */
}

