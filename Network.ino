#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

const char * ssid = "MT29501119";
const char * pass = "ErikaKot";

void net_setup() {
  WiFiMulti.addAP(ssid, pass);
  Serial.println("Connecting Wifi...");
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void net_register(float medianDistance) {
  StaticJsonBuffer<200> json;
  char jsonBuffer[256];

  JsonObject& root = json.createObject();
  root["id"] = ESP.getChipId();
  root["startup_distance"] = medianDistance;
  root.printTo(jsonBuffer, sizeof(jsonBuffer));

  http.begin("http://192.168.118.254:5000/sensor_register");
  http.POST(jsonBuffer);
  Serial.println(http.getString());
  http.end();
}

void net_report(int lastVal) {
  Serial.print("Ping: ");
  Serial.print(lastVal); // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.println("cm");

  /*
  StaticJsonBuffer<200> json;
  char jsonBuffer[256];

  JsonObject& root = json.createObject();
  root["id"] = ESP.getChipId();
  root["distance"] = lastVal;
  root["voltage"] = ESP.getVcc();
  root.printTo(jsonBuffer, sizeof(jsonBuffer));
  // root.printTo(Serial);

  http.begin("http://search-dungeon-qcc7an54euyyq4vnstm2j673ve.us-east-1.es.amazonaws.com/dungeon/sensor_record");
  http.POST(jsonBuffer);
  // Serial.println(http.getString());
  http.end();
  
  http.begin("http://192.168.118.254:5000/sensor_record");
  http.POST(jsonBuffer);
  // Serial.println(http.getString());
  http.end(); */
}

