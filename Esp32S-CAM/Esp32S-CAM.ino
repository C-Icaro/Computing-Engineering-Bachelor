/*
  esp32s-cam.ino
  Ponto único do firmware da ESP32-CAM.
  - Configure Wi-Fi (SSID/Senha)
  - Inicialize a câmera (pinos do módulo e PSRAM)
  - Inicie o servidor/stream acessível em /stream

  Observação: este é um esqueleto. Substitua pelo sketch funcional do seu projeto
  (por exemplo, migre aqui o que estiver em ProjetoM4.ino).
*/

void setup() {
  // Inicialização mínima para manter o arquivo válido.
  // Substitua pelo setup real (Wi-Fi, câmera e servidor /stream).
  // Serial.begin(115200);
}

void loop() {
  // Substitua pela lógica do seu firmware.
}

#include "esp_camera.h"
#include <esp_http_server.h>
#include <esp_timer.h>
#include <img_converters.h>
#include <WiFi.h>

// =================== Configurações Wi-Fi ===================
// Substitua pelos dados da sua rede 2.4 GHz
const char *WIFI_SSID = "Server";
const char *WIFI_PASS = "server123";

// =================== Configuração da câmera ===================
// Este código assume o módulo ESP32-CAM AI-Thinker
#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#else
#error "Defina os pinos para o seu modelo de câmera"
#endif

static void startCameraServer();

void configureCamera()
{
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
    config.frame_size = FRAMESIZE_VGA; // 640x480
    config.jpeg_quality = 18;          // maior valor = mais leve
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_QVGA; // 320x240
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Falha ao inicializar a câmera. Erro 0x%x\n", err);
    ESP.restart();
  }

  sensor_t *sensor = esp_camera_sensor_get();
  if (sensor == nullptr)
  {
    Serial.println("Falha ao obter sensor da câmera");
    ESP.restart();
  }

  // Ajustes opcionais da câmera
  sensor->set_brightness(sensor, 0); // -2 a 2
  sensor->set_contrast(sensor, 0);   // -2 a 2
  sensor->set_saturation(sensor, 0); // -2 a 2
  sensor->set_gainceiling(sensor, (gainceiling_t)GAINCEILING_2X);
  sensor->set_quality(sensor, 10); // 0-63 (menor é melhor)
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

void connectToWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.println();
  Serial.printf("Conectando-se a %s\n", WIFI_SSID);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
    if (++attempt > 60)
    {
      Serial.println("\nNão foi possível conectar ao Wi-Fi. Reiniciando...");
      ESP.restart();
    }
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println("Inicializando ESP32-CAM");

  configureCamera();
  connectToWiFi();
  startCameraServer();

  Serial.println("\nServidor da câmera iniciado");
  Serial.println("Use um navegador ou o script Python informado para visualizar a imagem.");
}

void loop()
{
  delay(100);
}

/********** Servidor da câmera baseado no exemplo oficial **********/

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
  }

  return res;
}

static esp_err_t status_handler(httpd_req_t *req)
{
  static char json_response[1024];
  sensor_t *sensor = esp_camera_sensor_get();
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
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    httpd_register_uri_handler(stream_httpd, &status_uri);
    Serial.println("Rotas ativas:");
    Serial.println("  /stream  -> MJPEG ao vivo");
    Serial.println("  /status  -> informações da câmera");
  }
  else
  {
    Serial.println("Falha ao iniciar servidor HTTP");
  }
}

