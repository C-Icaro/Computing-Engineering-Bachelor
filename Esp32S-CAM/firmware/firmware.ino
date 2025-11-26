/*
  esp32s-cam.ino
  Ponto único do firmware da ESP32-CAM.
  - Configure Wi-Fi (SSID/Senha)
  - Inicialize a câmera (pinos do módulo e PSRAM)
  - Inicie o servidor/stream acessível em /stream

  Observação: este é um esqueleto. Substitua pelo sketch funcional do seu projeto
  (por exemplo, migre aqui o que estiver em ProjetoM4.ino).
*/

// =================== Includes ===================
#include "esp_camera.h"
#include <esp_http_server.h>
#include <esp_timer.h>
#include <img_converters.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Tentar incluir WiFiClientSecure, se não estiver disponível usar WiFiClient
#ifdef ESP32
  #include <WiFiClientSecure.h>
#else
  #include <WiFiClient.h>
  typedef WiFiClient WiFiClientSecure;
#endif

#include <ArduinoJson.h>

// =================== Módulos do projeto ===================
#include "config.h"
#include "utils.h"
#include "WiFiConnector.h"
#include "CameraController.h"
#include "YoloController.h"
#include "MQTTPublisher.h"
#include "http_server.h"

// =================== Instâncias globais ===================
WiFiConnector wifiConnector;
CameraController cameraController;
YoloController yoloController;
MQTTPublisher mqttPublisher;

// =================== Setup ===================
void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println("Inicializando ESP32-CAM");

  if (!cameraController.begin())
  {
    Serial.println("[Setup] Falha ao inicializar câmera. Reiniciando...");
    ESP.restart();
  }

  if (!wifiConnector.connect(WIFI_SSID, WIFI_PASS))
  {
    Serial.println("[Setup] Wi-Fi indisponível. Reiniciando...");
    ESP.restart();
  }

  yoloController.begin(YOLO_INFERENCE_ENDPOINT);
  
  // Inicializar MQTT
  if (mqttPublisher.begin())
  {
    Serial.println("[MQTT] Inicializado. Tentando conectar...");
  }
  else
  {
    Serial.println("[MQTT] Desabilitado ou falha na inicialização");
  }
  
  startCameraServer();

  Serial.println("\n========================================");
  Serial.println("  Sistema Pronto!");
  Serial.println("========================================");
  Serial.println("Servidor HTTP: http://" + WiFi.localIP().toString());
  Serial.println("Stream MJPEG: http://" + WiFi.localIP().toString() + "/stream");
  if (mqttEnabled)
  {
    Serial.println("MQTT Broker: " + String(MQTT_BROKER));
    Serial.println("Tópico Frames: " + String(MQTT_TOPIC_FRAMES));
    Serial.println("Tópico Comandos: " + String(MQTT_TOPIC_COMMANDS));
  }
  Serial.println("========================================\n");
}

// =================== Loop principal ===================
void loop()
{
  // Manter conexão MQTT ativa e processar mensagens
  mqttPublisher.loop();
  
  delay(10); // Pequeno delay para não sobrecarregar
}
