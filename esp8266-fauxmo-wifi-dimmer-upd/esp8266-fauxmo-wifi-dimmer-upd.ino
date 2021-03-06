#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <fauxmoESP.h>
#include <WiFiUdp.h>

#include "settings.h"

fauxmoESP fauxmo;
WiFiUDP Udp;
ESP8266WebServer httpServer(8080);
ESP8266HTTPUpdateServer httpUpdater;

#define SERIAL_BAUDRATE 115200

#ifndef WLAN_SSID
#define WLAN_SSID "SSID"
#endif

#ifndef WLAN_PSK
#define WLAN_PSK "0123456789"
#endif

#ifndef ESP_DEVICE_IP
#define ESP_DEVICE_IP "192.168.0.123"
#endif

#ifndef ESP_DIMMER_ID
#define ESP_DIMMER_ID "ESP Fauxmo Device"
#endif

const char* ssid = WLAN_SSID;
const char* password = WLAN_PSK;
const char* esp_device_id = ESP_DEVICE_IP;
unsigned long previousMillis = 0;
unsigned long interval = 1000;
int ledState = LOW;

void wifi() {

  WiFi.mode(WIFI_STA);

  int connectionRetryCounter = 0;

  Serial.print("[APP] Trying to connect to WiFi");

  WiFi.begin(ssid, password);

  // try a few times times (the timeout sometimes does not work for me)
  while (WiFi.waitForConnectResult() != WL_CONNECTED && connectionRetryCounter < 15) {
    connectionRetryCounter += 1;
    Serial.print(".");
    delay(100);
  }

  Serial.println();

  if (connectionRetryCounter >= 15) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP", "0123456789");
    Serial.printf("[WIFI] AP Mode, SSID: %s, IP address: %s", WiFi.SSID().c_str(), WiFi.softAPIP().toString().c_str());
  } else {
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  }

  Serial.println();

}

void setup() {

  Serial.begin(SERIAL_BAUDRATE);
  Serial.println();

  wifi();

  Udp.begin(18666);

  fauxmo.createServer(true);
  fauxmo.setPort(80);
  fauxmo.enable(true);

  fauxmo.addDevice(ESP_DIMMER_ID);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {

    Serial.printf("[FAUXMO] Device #%d (%s) state: %s value: %d", device_id, device_name, state ? "ON" : "OFF", value);
    Serial.println();

    if (strcmp(device_name, ESP_DIMMER_ID) == 0) {

      if (state == HIGH) {

        Udp.beginPacket(esp_device_id, 18566);
        Udp.write("4;1");
        Udp.endPacket();

        Serial.printf("[UDP] '4;1' send to %s", esp_device_id);
        Serial.println();

        String dimmerValue = "16;" + String(value);
        Udp.beginPacket("192.168.1.192", 18566);
        Udp.write(dimmerValue.c_str());
        Udp.endPacket();

        Serial.printf("[UDP] '%s' send to %s", dimmerValue.c_str(), esp_device_id);
        Serial.println();

      } else {

        Udp.beginPacket("192.168.1.192", 18566);
        Udp.write("4;0");
        Udp.endPacket();
        Serial.printf("[UDP] '4;0' send to %s", esp_device_id);
        Serial.println();
        
        if (value == 1) { // null percent used to reboot module
          Udp.beginPacket("192.168.1.192", 18566);
          Udp.write("2");
          Udp.endPacket();
          Serial.printf("[UDP] '2' (reset) send to %s", esp_device_id);
          Serial.println();          
        }
       
      }

    }

  });

  httpUpdater.setup(&httpServer);
  httpServer.begin();

}

void loop() {

  httpServer.handleClient();

  fauxmo.handle();

  // This is a sample code to output free heap every 5 seconds
  // This is a cheap way to detect memory leaks
  static unsigned long last = millis();
  if (millis() - last > 5000) {
    last = millis();
    Serial.printf("[MAIN] Free heap: %d bytes", ESP.getFreeHeap());
    Serial.println();
  }

}
