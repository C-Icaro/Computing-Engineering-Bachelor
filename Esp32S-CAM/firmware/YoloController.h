#ifndef YOLO_CONTROLLER_H
#define YOLO_CONTROLLER_H

#include "esp_camera.h"
#include <Arduino.h>

class YoloController
{
public:
  void begin(const String &endpoint = "")
  {
    inferenceEndpoint = endpoint;
    enabled = false;
  }

  void setEnabled(bool value)
  {
    enabled = value;
    Serial.printf("[YOLO] Detecção %s\n", enabled ? "ativada" : "desativada");
  }

  bool isEnabled() const
  {
    return enabled;
  }

  void toggle()
  {
    setEnabled(!enabled);
  }

  void setEndpoint(const String &endpoint)
  {
    inferenceEndpoint = endpoint;
  }

  const String &getEndpoint() const
  {
    return inferenceEndpoint;
  }

  void processFrame(camera_fb_t *fb)
  {
    if (!enabled || fb == nullptr)
    {
      return;
    }

    const unsigned long now = millis();
    if (now - lastLogMillis >= 2000)
    {
      Serial.printf("[YOLO] Frame pronto (%u bytes). Integre seu pipeline YOLO aqui.\n", fb->len);
      if (inferenceEndpoint.length() > 0)
      {
        Serial.printf("[YOLO] Endpoint configurado: %s (implementação da chamada HTTP não inclusa).\n", inferenceEndpoint.c_str());
      }
      lastLogMillis = now;
    }
  }

private:
  bool enabled = false;
  String inferenceEndpoint;
  unsigned long lastLogMillis = 0;
};

#endif // YOLO_CONTROLLER_H


