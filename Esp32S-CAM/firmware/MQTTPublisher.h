#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "config.h"
#include "utils.h"
#include "esp_camera.h"
#include "OperationMode.h"

// Tentar incluir WiFiClientSecure, se não estiver disponível usar WiFiClient
#ifdef ESP32
  #include <WiFiClientSecure.h>
#else
  #include <WiFiClient.h>
  typedef WiFiClient WiFiClientSecure;
#endif

// Forward declarations
class OperationModeController;

// Callback externo para mudança de modo (definido em firmware.ino)
extern void onModeChangedCallback(OperationMode newMode);

// Callback externo para status do PIR (definido em firmware.ino)
extern void onPIRStatusRequestedCallback();

class MQTTPublisher
{
public:
  MQTTPublisher() : client(espClient) {}

  bool begin()
  {
    if (!mqttEnabled)
    {
      Serial.println("[MQTT] MQTT desabilitado nas configurações");
      return false;
    }

    // Configurar buffer para frames
    // PubSubClient tem limite máximo de ~256KB na maioria das versões
    // Tentar valores menores primeiro
    bool bufferSet = false;
    int bufferSizes[] = {128 * 1024, 64 * 1024, 32 * 1024}; // 128KB, 64KB, 32KB
    
    for (int i = 0; i < 3; i++) {
      if (client.setBufferSize(bufferSizes[i])) {
        Serial.printf("[MQTT] Buffer configurado: %d KB\n", bufferSizes[i] / 1024);
        bufferSet = true;
        break;
      }
    }
    
    if (!bufferSet) {
      Serial.println("[MQTT] AVISO: Usando buffer padrão (~256 bytes)");
      Serial.println("[MQTT] Frames grandes podem falhar. Considere atualizar PubSubClient.");
    }
    
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback([this](char *topic, byte *payload, unsigned int length) {
      this->onMessage(topic, payload, length);
    });

    Serial.println("[MQTT] Configurado. Broker: " + String(MQTT_BROKER));
    Serial.println("[MQTT] Buffer configurado: 256KB");
    return true;
  }

  void loop()
  {
    if (!mqttEnabled || !client.connected())
    {
      if (mqttEnabled)
      {
        tryReconnect();
      }
      return;
    }

    client.loop();
  }

  bool publishFrame(camera_fb_t *fb, const String &mode = "", bool motionTriggered = false)
  {
    if (!mqttEnabled || !client.connected() || fb == nullptr)
    {
      return false;
    }

    // Verificar se o frame é muito grande
    if (fb->len > MQTT_MAX_FRAME_SIZE)
    {
      Serial.printf("[MQTT] Frame muito grande (%u bytes > %u), pulando...\n", 
                    fb->len, MQTT_MAX_FRAME_SIZE);
      return false;
    }
    
    // Ajustar qualidade do sensor
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor)
    {
      sensor->set_quality(sensor, MQTT_JPEG_QUALITY);
    }

    // Processar frame diretamente na RAM
    return publishFrameDirect(fb, mode, motionTriggered);
  }

  void publishStatus(const String &status)
  {
    if (!mqttEnabled || !client.connected())
    {
      return;
    }

    DynamicJsonDocument doc(512);
    doc["timestamp"] = millis();
    doc["status"] = status;
    doc["ip"] = WiFi.localIP().toString();
    doc["uptime"] = millis() / 1000;

    String jsonPayload;
    serializeJson(doc, jsonPayload);
    client.publish(MQTT_TOPIC_STATUS, jsonPayload.c_str());
  }

  void publishMotionDetected()
  {
    if (!mqttEnabled || !client.connected())
    {
      return;
    }

    DynamicJsonDocument doc(256);
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["timestamp"] = millis();
    doc["motion_detected"] = true;

    String jsonPayload;
    serializeJson(doc, jsonPayload);
    client.publish(MQTT_TOPIC_MOTION, jsonPayload.c_str());
    Serial.println("[MQTT] Notificação de movimento publicada");
  }

  void publishMode(const String &mode)
  {
    if (!mqttEnabled || !client.connected())
    {
      return;
    }

    DynamicJsonDocument doc(256);
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["timestamp"] = millis();
    doc["mode"] = mode;

    String jsonPayload;
    serializeJson(doc, jsonPayload);
    client.publish(MQTT_TOPIC_MODE, jsonPayload.c_str());
    Serial.printf("[MQTT] Modo publicado: %s\n", mode.c_str());
  }


  // Callback para captura manual (será chamado externamente)
  void requestManualCapture()
  {
    manualCaptureRequested = true;
  }

  bool isManualCaptureRequested()
  {
    if (manualCaptureRequested)
    {
      manualCaptureRequested = false; // Reset flag após ler
      return true;
    }
    return false;
  }

  bool isConnected()
  {
    return client.connected();
  }

  void setEnabled(bool enabled)
  {
    mqttEnabled = enabled;
    if (!enabled && client.connected())
    {
      client.disconnect();
    }
  }

private:
  // Método para processamento direto na RAM
  bool publishFrameDirect(camera_fb_t *fb, const String &mode, bool motionTriggered) {
    size_t base64Size = ((fb->len + 2) / 3) * 4;
    size_t jsonSize = base64Size + 300;
    
    DynamicJsonDocument doc(jsonSize);
    doc["device_id"] = MQTT_CLIENT_ID;
    doc["frame_id"] = frameCounter++;
    doc["timestamp"] = millis();
    doc["format"] = "jpeg";
    doc["width"] = fb->width;
    doc["height"] = fb->height;
    doc["size"] = fb->len;
    doc["quality"] = MQTT_JPEG_QUALITY;
    
    if (mode.length() > 0)
    {
      doc["mode"] = mode;
    }
    
    doc["motion_triggered"] = motionTriggered;

    String base64Frame;
    base64EncodeChunk(fb->buf, fb->len, base64Frame);
    doc["data"] = base64Frame;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    bool result = client.publish(MQTT_TOPIC_FRAMES, jsonPayload.c_str());
    
    if (result) {
      Serial.printf("[MQTT] ✓ Frame publicado: JPEG=%u, JSON=%u bytes, Mode=%s, Motion=%s\n", 
                    fb->len, jsonPayload.length(), mode.c_str(), motionTriggered ? "true" : "false");
    }

    return result;
  }

  String modeToString(OperationMode mode) const
  {
    switch (mode)
    {
      case OperationMode::HIBERNATION:
        return "hibernation";
      case OperationMode::AUTO:
        return "auto";
      case OperationMode::MANUAL:
        return "manual";
      case OperationMode::VIGILANCE:
        return "vigilance";
      default:
        return "unknown";
    }
  }

  WiFiClientSecure espClient;  // Deve vir antes de client
  PubSubClient client;
  unsigned long lastReconnectAttempt = 0;
  const unsigned long RECONNECT_INTERVAL = 10000; // 10 segundos
  uint32_t frameCounter = 0;
  bool manualCaptureRequested = false; // Flag para captura manual

  void onMessage(char *topic, byte *payload, unsigned int length)
  {
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
      message += (char)payload[i];
    }

    Serial.printf("[MQTT] Mensagem recebida [%s]: %s\n", topic, message.c_str());

    if (String(topic) == MQTT_TOPIC_COMMANDS)
    {
      handleCommand(message);
    }
  }

  void handleCommand(const String &command)
  {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, command);

    if (error)
    {
      Serial.println("[MQTT] Erro ao parsear comando JSON");
      return;
    }

    if (doc.containsKey("action"))
    {
      String action = doc["action"].as<String>();

      if (action == "set_mode")
      {
        if (doc.containsKey("mode"))
        {
          String modeStr = doc["mode"].as<String>();
          OperationMode newMode = OperationModeController::stringToMode(modeStr);
          // Chamar callback externo para mudar o modo
          onModeChangedCallback(newMode);
          Serial.printf("[MQTT] Comando de mudança de modo recebido: %s\n", modeStr.c_str());
        }
      }
      else if (action == "capture")
      {
        requestManualCapture();
        Serial.println("[MQTT] Comando de captura manual recebido");
      }
      else if (action == "toggle_mqtt")
      {
        bool enabled = doc.containsKey("enabled") ? doc["enabled"].as<bool>() : !mqttEnabled;
        setEnabled(enabled);
        Serial.printf("[MQTT] MQTT %s via comando remoto\n", enabled ? "ativado" : "desativado");
      }
      else if (action == "restart")
      {
        Serial.println("[MQTT] Reiniciando ESP32 via comando remoto...");
        delay(1000);
        ESP.restart();
      }
      else if (action == "pir_status")
      {
        // Chamar callback externo para imprimir status do PIR
        onPIRStatusRequestedCallback();
        Serial.println("[MQTT] Comando de status do PIR recebido");
      }
    }
  }

  void tryReconnect()
  {
    unsigned long now = millis();
    if (now - lastReconnectAttempt < RECONNECT_INTERVAL)
    {
      return;
    }

    lastReconnectAttempt = now;

    Serial.print("[MQTT] Tentando conectar ao broker...");

    // Configurar certificado TLS (HiveMQ usa certificado válido)
    #ifdef ESP32
      espClient.setInsecure(); // Aceita qualquer certificado (para desenvolvimento)
    #endif

    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS))
    {
      Serial.println(" Conectado!");

      // Subscrever ao tópico de comandos
      if (client.subscribe(MQTT_TOPIC_COMMANDS))
      {
        Serial.println("[MQTT] Inscrito em: " + String(MQTT_TOPIC_COMMANDS));
      }

      // Publicar status inicial
      publishStatus("online");
    }
    else
    {
      Serial.print(" Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 10 segundos...");
    }
  }
};

#endif // MQTT_PUBLISHER_H

