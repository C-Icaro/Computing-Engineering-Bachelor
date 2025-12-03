#ifndef WIFI_CONNECTOR_H
#define WIFI_CONNECTOR_H

#include <WiFi.h>
#include <Arduino.h>

class WiFiConnector
{
public:
  bool connect(const char *ssid, const char *pass, uint32_t timeoutMs = 30000UL)
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    Serial.println();
    Serial.printf("[WiFi] Conectando-se a %s\n", ssid);

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print('.');
      if (millis() - start > timeoutMs)
      {
        Serial.println("\n[WiFi] Tempo limite excedido. Reiniciando...");
        return false;
      }
    }

    Serial.println("\n[WiFi] Conectado!");
    Serial.print("[WiFi] Endereço IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
};

#endif // WIFI_CONNECTOR_H


