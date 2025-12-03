#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <esp_http_server.h>
#include <img_converters.h>
#include "esp_camera.h"
#include "CameraController.h"
#include "MQTTPublisher.h"
#include <Arduino.h>

// Declarações externas das instâncias globais
extern CameraController cameraController;
extern MQTTPublisher mqttPublisher;

// HTML da interface web
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>ESP32-CAM Vision</title>
  <style>
    :root {
      color-scheme: dark;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    }
    body {
      margin: 0;
      padding: 0;
      background: #0f172a;
      color: #f8fafc;
    }
    .container {
      max-width: 960px;
      margin: 0 auto;
      padding: 32px 20px 60px;
    }
    h1 {
      margin-bottom: 8px;
      font-size: 2rem;
      text-align: center;
    }
    .subtitle {
      text-align: center;
      margin-bottom: 24px;
      opacity: 0.75;
    }
    .video-box {
      background: #1e293b;
      border-radius: 16px;
      padding: 12px;
      box-shadow: 0 20px 60px rgba(15, 23, 42, 0.45);
    }
    .video-box img {
      width: 100%;
      border-radius: 12px;
      background: #000;
      min-height: 240px;
      object-fit: cover;
    }
    .controls {
      margin-top: 20px;
      display: flex;
      flex-wrap: wrap;
      gap: 16px;
      align-items: center;
      justify-content: center;
    }
    button {
      border: none;
      padding: 12px 28px;
      border-radius: 999px;
      font-size: 1rem;
      cursor: pointer;
      background: linear-gradient(135deg, #38bdf8, #6366f1);
      color: #fff;
      font-weight: 600;
      transition: transform 0.15s ease, opacity 0.15s ease;
    }
    button:hover {
      transform: translateY(-2px);
      opacity: 0.95;
    }
    .badge {
      padding: 10px 18px;
      border-radius: 999px;
      font-weight: 600;
    }
    .badge.on {
      background: rgba(34, 197, 94, 0.2);
      color: #34d399;
      border: 1px solid rgba(34, 197, 94, 0.35);
    }
    .badge.off {
      background: rgba(248, 113, 113, 0.2);
      color: #f87171;
      border: 1px solid rgba(248, 113, 113, 0.35);
    }
    .info-card {
      margin-top: 24px;
      background: rgba(30, 41, 59, 0.85);
      border-radius: 16px;
      padding: 20px;
      line-height: 1.6;
    }
    @media (max-width: 600px) {
      .container {
        padding: 20px 16px 40px;
      }
      button {
        width: 100%;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32-CAM Monitoramento Ferroviário</h1>
    <p class="subtitle">Sistema de monitoramento de via férrea com detecção de movimento.</p>
    <div class="video-box">
      <img id="stream" src="/stream" alt="Live stream" />
    </div>
    <div class="controls">
      <span id="statusBadge" class="badge off">Status: Desconhecido</span>
    </div>
    <div class="info-card">
      <strong>Sistema:</strong> Monitoramento de via férrea com sensor PIR<br />
      <strong>Modo:</strong> Controlado via MQTT<br />
      <small>Use o servidor CCO para análise de IA e detecção de objetos.</small>
    </div>
  </div>
  <script>
    async function fetchStatus() {
      try {
        const res = await fetch('/status');
        const data = await res.json();
        const badge = document.getElementById('statusBadge');
        badge.textContent = 'Status: Online';
        badge.classList.add('on');
        badge.classList.remove('off');
      } catch (err) {
        console.error('Falha ao obter status', err);
        const badge = document.getElementById('statusBadge');
        badge.textContent = 'Status: Offline';
        badge.classList.add('off');
        badge.classList.remove('on');
      }
    }
    window.addEventListener('load', () => {
      fetchStatus();
      setInterval(fetchStatus, 10000);
    });
  </script>
</body>
</html>
)rawliteral";

// Handlers HTTP
static esp_err_t root_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t stream_handler(httpd_req_t *req)
{
  camera_fb_t *fb = nullptr;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = nullptr;
  char part_buf[64];

  res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
  if (res != ESP_OK)
  {
    return res;
  }

  while (true)
  {
    fb = esp_camera_fb_get();
    if (!fb)
    {
      Serial.println("Falha ao obter frame da câmera");
      res = ESP_FAIL;
    }
    else
    {
      if (fb->format != PIXFORMAT_JPEG)
      {
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return(fb);
        fb = nullptr;
        if (!jpeg_converted)
        {
          Serial.println("Falha ao converter frame para JPEG");
          res = ESP_FAIL;
        }
      }
      else
      {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }

    if (res == ESP_OK)
    {
      // Enviar frame via HTTP stream
      size_t hlen = (size_t)snprintf(part_buf, sizeof(part_buf), "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", (unsigned int)_jpg_buf_len);
      res = httpd_resp_send_chunk(req, part_buf, hlen);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, "\r\n", 2);
    }

    // Publicar frame via MQTT APÓS enviar HTTP (não bloqueia stream)
    // Fazer isso antes de retornar o frame para não perder dados
    if (res == ESP_OK && mqttPublisher.isConnected() && fb != nullptr)
    {
      // Criar cópia do frame para MQTT (para não bloquear retorno do frame)
      // Mas como isso consome memória, vamos processar MQTT de forma assíncrona
      mqttPublisher.publishFrame(fb);
    }

    // Retornar frame IMEDIATAMENTE para evitar overflow
    if (fb)
    {
      esp_camera_fb_return(fb);
      fb = nullptr;
      _jpg_buf = nullptr;
    }
    else if (_jpg_buf)
    {
      free(_jpg_buf);
      _jpg_buf = nullptr;
    }

    if (res != ESP_OK)
    {
      break;
    }
    
    // Pequeno delay para evitar sobrecarga
    delay(10);
  }

  return res;
}

static esp_err_t status_handler(httpd_req_t *req)
{
  static char json_response[1024];
  sensor_t *sensor = cameraController.getSensor();
  if (!sensor)
  {
    return httpd_resp_send_500(req);
  }

  char *p = json_response;
  *p++ = '{';
  p += sprintf(p, "\"framesize\":%u,", sensor->status.framesize);
  p += sprintf(p, "\"quality\":%u,", sensor->status.quality);
  p += sprintf(p, "\"brightness\":%d,", sensor->status.brightness);
  p += sprintf(p, "\"contrast\":%d,", sensor->status.contrast);
  p += sprintf(p, "\"saturation\":%d", sensor->status.saturation);
  *p++ = '}';
  *p++ = '\0';

  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, json_response, strlen(json_response));
}

void startCameraServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.ctrl_port = 32768;

  httpd_uri_t root_uri = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = root_handler,
      .user_ctx = nullptr};

  httpd_uri_t stream_uri = {
      .uri = "/stream",
      .method = HTTP_GET,
      .handler = stream_handler,
      .user_ctx = nullptr};

  httpd_uri_t status_uri = {
      .uri = "/status",
      .method = HTTP_GET,
      .handler = status_handler,
      .user_ctx = nullptr};

  httpd_handle_t stream_httpd = nullptr;

  if (httpd_start(&stream_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(stream_httpd, &root_uri);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    httpd_register_uri_handler(stream_httpd, &status_uri);
    Serial.println("Rotas ativas:");
    Serial.println("  /stream  -> MJPEG ao vivo");
    Serial.println("  /status  -> informações da câmera");
    Serial.println("  /        -> painel web interativo");
  }
  else
  {
    Serial.println("Falha ao iniciar servidor HTTP");
  }
}

#endif // HTTP_SERVER_H


