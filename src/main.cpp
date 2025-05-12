#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
  #include <WiFiMulti.h>
  #include <WebServer.h>
  #include <Update.h>
  WebServer server(80);
  WiFiMulti wifiMulti;  
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <Updater.h>  // alternativní název v některých core verzích
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
#endif

#include <ArduinoOTA.h>
#include "config.h"
#include "htmlweb.h"
#include "version.h"



String htmlAdmin;
String chipIdHtml;

void handleAdmin() {
  server.sendHeader("Content-Type", "text/html; charset=utf-8");
  htmlAdmin = htmlAdminTemplate;
  htmlAdmin.replace("%CHIP_ID%", chipIdHtml);
  htmlAdmin.replace("%BUILD_VERSION%", BUILD_VERSION);
  server.send(200, "text/html", htmlAdmin);
}

void handleOTAUpdate() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    #if defined(ESP32)
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
    #elif defined(ESP8266)
      if (!Update.begin((size_t)0)) Update.printError(Serial);
    #endif
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
      Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      server.send(200, "text/plain", "OK");
      delay(1000);
      ESP.restart();
    } else {
      Update.printError(Serial);
      server.send(500, "text/plain", "Update failed");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // zajistí, že první výpisy nejsou ztraceny

  Serial.println("Spoustím ESP...");

  #if defined(ESP32)
    uint64_t chipId = ESP.getEfuseMac();
    char chipIdStr[13];
    sprintf(chipIdStr, "%04X", (uint16_t)(chipId >> 32));
  #elif defined(ESP8266)
    uint32_t chipId = ESP.getChipId();
    char chipIdStr[13];
    sprintf(chipIdStr, "%04X", chipId & 0xFFFF);
  #endif

  chipIdHtml = String(chipIdStr);

  Serial.println("Nastavuji režim WIFI_AP_STA...");
 
  
  Serial.println("Nastavuji režim WIFI_AP_STA...");
  WiFi.mode(WIFI_AP_STA);

  Serial.printf("Vytvářím AP: %s\n", SOFTAP_SSID_1);
  WiFi.softAP(SOFTAP_SSID_1, SOFTAP_PASS_1);
  Serial.print("IP adresa AP: ");
  Serial.println(WiFi.softAPIP());

  #if defined(ESP32)
    wifiMulti.addAP(SSID_1, PASS_1);
    WiFi.setSleep(false);
  #elif defined(ESP8266)
    WiFi.disconnect();  // bez true - nesmaže přihlašovací údaje
    delay(100);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin(SSID_1, PASS_1);
  #endif




  #if defined(ESP32)
    WiFi.setSleep(false);
  #endif

  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 20000;

  Serial.printf("Připojuji se k Wi-Fi: %s\n", SSID_1);

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    #if defined(ESP32)
      wifiMulti.run();
    #endif
    Serial.print(".");
    delay(500);
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi připojeno!");
    Serial.print("IP adresa STA: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Nepodařilo se připojit k Wi-Fi.");
    Serial.print("Status: ");
    Serial.println(WiFi.status());
  }


  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi připojeno!");
    Serial.print("IP adresa STA: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNepodařilo se připojit k Wi-Fi.");
  }

  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();
  Serial.println("OTA aktivováno.");

  server.on("/", handleAdmin);
  server.on("/admin", handleAdmin);
  server.on("/update", HTTP_POST, []() {}, handleOTAUpdate);
  server.begin();
  Serial.println("HTTP server spuštěn.");

  Serial.printf("ChipID: %s\n", chipIdStr);
  Serial.printf("Firmware verze: %s\n", BUILD_VERSION);
}


void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
