#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "esp_camera.h"
#include "config.h"
#include <Arduino.h>

class CameraController
{
public:
  CameraController() : initialized(false) {}

  bool begin()
  {
    if (initialized)
    {
      return true; // Já inicializada
    }

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound())
    {
      config.frame_size = FRAMESIZE_VGA;
      config.jpeg_quality = 18;
      config.fb_count = 2; // 2 buffers para evitar overflow
      Serial.println("[Camera] PSRAM detectado - usando VGA com 2 buffers");
    }
    else
    {
      config.frame_size = FRAMESIZE_QVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
      Serial.println("[Camera] Sem PSRAM - usando QVGA com 1 buffer");
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
      Serial.printf("[Camera] Falha ao inicializar. Erro 0x%x\n", err);
      return false;
    }

    applyDefaultTuning();
    initialized = true;
    return true;
  }

  bool isInitialized() const
  {
    return initialized;
  }

  void powerOn()
  {
    // PWDN = LOW ativa a câmera
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, LOW);
    delay(CAMERA_STABILIZATION_MS);
    Serial.println("[Camera] Câmera ligada");
  }

  void powerOff()
  {
    // PWDN = HIGH desativa a câmera
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, HIGH);
    delay(50);
    Serial.println("[Camera] Câmera desligada");
  }

  camera_fb_t* captureFrame()
  {
    if (!initialized)
    {
      return nullptr;
    }
    return esp_camera_fb_get();
  }

  void returnFrame(camera_fb_t* fb)
  {
    if (fb != nullptr)
    {
      esp_camera_fb_return(fb);
    }
  }

  sensor_t *getSensor()
  {
    return esp_camera_sensor_get();
  }

  void applyDefaultTuning()
  {
    sensor_t *sensor = getSensor();
    if (sensor == nullptr)
    {
      Serial.println("[Camera] Falha ao obter sensor para ajustes.");
      return;
    }

    sensor->set_brightness(sensor, 0);
    sensor->set_contrast(sensor, 0);
    sensor->set_saturation(sensor, 0);
    sensor->set_gainceiling(sensor, (gainceiling_t)GAINCEILING_2X);
    sensor->set_quality(sensor, 10);
    sensor->set_framesize(sensor, FRAMESIZE_VGA);
    sensor->set_whitebal(sensor, 1);
    sensor->set_awb_gain(sensor, 1);
    sensor->set_wb_mode(sensor, 0);
    sensor->set_exposure_ctrl(sensor, 1);
    sensor->set_aec2(sensor, 0);
    sensor->set_ae_level(sensor, 0);
    sensor->set_aec_value(sensor, 300);
    sensor->set_gain_ctrl(sensor, 1);
    sensor->set_agc_gain(sensor, 0);
    sensor->set_bpc(sensor, 0);
    sensor->set_wpc(sensor, 1);
    sensor->set_lenc(sensor, 1);
    sensor->set_hmirror(sensor, 0);
    sensor->set_vflip(sensor, 0);
    sensor->set_dcw(sensor, 1);
    sensor->set_colorbar(sensor, 0);
  }

private:
  bool initialized;
};

#endif // CAMERA_CONTROLLER_H


