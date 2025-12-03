#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "esp_camera.h"

class PowerManager
{
public:
  PowerManager() : cameraActive(false) {}

  void enterHibernation()
  {
    if (cameraActive)
    {
      powerOff();
    }
    Serial.println("[Power] Modo hibernação ativado - câmera desligada");
  }

  void wakeUp()
  {
    if (!cameraActive)
    {
      powerOn();
    }
    Serial.println("[Power] Sistema acordado - câmera ligada");
  }

  bool isCameraActive() const
  {
    return cameraActive;
  }

  void powerOn()
  {
    if (cameraActive)
    {
      return; // Já está ligada
    }

    // PWDN = LOW ativa a câmera
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, LOW);
    delay(100); // Aguardar estabilização

    cameraActive = true;
    Serial.println("[Power] Câmera ligada");
  }

  void powerOff()
  {
    if (!cameraActive)
    {
      return; // Já está desligada
    }

    // Retornar todos os frames antes de desligar
    esp_camera_fb_return(esp_camera_fb_get());

    // PWDN = HIGH desativa a câmera
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, HIGH);
    delay(50);

    cameraActive = false;
    Serial.println("[Power] Câmera desligada");
  }

private:
  bool cameraActive;
};

#endif // POWER_MANAGER_H




