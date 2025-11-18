/*
  esp32s-cam.ino
  Ponto único do firmware da ESP32-CAM.
  - Configure Wi-Fi (SSID/Senha)
  - Inicialize a câmera (pinos do módulo e PSRAM)
  - Inicie o servidor/stream acessível em /stream

  Observação: este é um esqueleto. Substitua pelo sketch funcional do seu projeto
  (por exemplo, migre aqui o que estiver em ProjetoM4.ino).
*/

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
// Base64 será implementado inline
// Função auxiliar para codificação base64 em chunks (economiza RAM)
void base64EncodeChunk(const uint8_t* data, size_t length, String& result) {
  const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  result = "";
  result.reserve(((length + 2) / 3) * 4); // Pré-alocar espaço
  
  size_t i = 0;
  while (i < length) {
    uint32_t octet_a = i < length ? data[i++] : 0;
    uint32_t octet_b = i < length ? data[i++] : 0;
    uint32_t octet_c = i < length ? data[i++] : 0;
    
    uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;
    
    result += base64_chars[(triple >> 18) & 0x3F];
    result += base64_chars[(triple >> 12) & 0x3F];
    result += (i - 2 < length) ? base64_chars[(triple >> 6) & 0x3F] : '=';
    result += (i - 1 < length) ? base64_chars[triple & 0x3F] : '=';
  }
}


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

// =================== Configurações Wi-Fi ===================
// Substitua pelos dados da sua rede 2.4 GHz
const char *WIFI_SSID = "Server";
const char *WIFI_PASS = "server123";

// =================== Configuração de YOLO ===================
// Caso possua um endpoint HTTP para inferência YOLO (por exemplo, um servidor Python),
// informe abaixo. Caso contrário, deixe vazio para utilizar apenas o placeholder local.
const char *YOLO_INFERENCE_ENDPOINT = "";

// =================== Configuração MQTT (HiveMQ Cloud) ===================
// PREENCHA COM SUAS CREDENCIAIS DO HIVEMQ CLOUD:
// 1. Acesse: https://www.hivemq.com/mqtt-cloud-broker/
// 2. Crie uma conta gratuita e um cluster
// 3. Copie as credenciais abaixo

const char *MQTT_BROKER = "7bf2160d4f4849f0885901cae207a260.s1.eu.hivemq.cloud";  // Ex: abc123.s1.eu.hivemq.cloud
const int MQTT_PORT = 8883;                                    // Porta TLS
const char *MQTT_USER = "esp32cam_001";                        // Seu username do HiveMQ
const char *MQTT_PASS = "Esp32cam_001";                          // Sua senha do HiveMQ
const char *MQTT_CLIENT_ID = "esp32cam_device_001";                  // ID único do cliente

// Tópicos MQTT
const char *MQTT_TOPIC_FRAMES = "esp32cam/frames";            // Publica frames aqui
const char *MQTT_TOPIC_STATUS = "esp32cam/status";            // Publica status aqui
const char *MQTT_TOPIC_COMMANDS = "esp32cam/commands";         // Recebe comandos aqui

// Configurações de publicação
const unsigned long MQTT_PUBLISH_INTERVAL = 2000;             // Publica a cada 2 segundos (0.5 FPS)
const int MQTT_JPEG_QUALITY = 12;                              // Qualidade JPEG (1-63, menor = melhor) - reduzido para frames menores
const int MQTT_MAX_FRAME_SIZE = 20000;                        // Tamanho máximo do frame JPEG em bytes (20KB) - reduzido para evitar problemas de memória
bool mqttEnabled = true;                                       // Ativar/desativar MQTT

// =================== Utilitários e Controladores ===================

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

class CameraController
{
public:
  bool begin()
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
    return true;
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
};

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

// Declaração forward
class YoloController;
extern YoloController yoloController; // Declaração externa da instância global

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

  bool publishFrame(camera_fb_t *fb)
  {
    if (!mqttEnabled || !client.connected() || fb == nullptr)
    {
      return false;
    }

    static unsigned long lastPublish = 0;
    unsigned long now = millis();

    if (now - lastPublish < MQTT_PUBLISH_INTERVAL)
    {
      return false; // Ainda não passou o intervalo
    }

    lastPublish = now;

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
    return publishFrameDirect(fb);
  }

private:
  // Método para processamento direto na RAM
  bool publishFrameDirect(camera_fb_t *fb) {
    size_t base64Size = ((fb->len + 2) / 3) * 4;
    size_t jsonSize = base64Size + 200;
    
    DynamicJsonDocument doc(jsonSize);
    doc["timestamp"] = millis();
    doc["frame_id"] = frameCounter++;
    doc["format"] = "jpeg";
    doc["width"] = fb->width;
    doc["height"] = fb->height;
    doc["size"] = fb->len;
    doc["quality"] = MQTT_JPEG_QUALITY;

    String base64Frame;
    base64EncodeChunk(fb->buf, fb->len, base64Frame);
    doc["data"] = base64Frame;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    bool result = client.publish(MQTT_TOPIC_FRAMES, jsonPayload.c_str());
    
    if (result) {
      Serial.printf("[MQTT] ✓ Frame publicado (direto): JPEG=%u, JSON=%u bytes\n", 
                    fb->len, jsonPayload.length());
    }

    return result;
  }

public:

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
  WiFiClientSecure espClient;  // Deve vir antes de client
  PubSubClient client;
  unsigned long lastReconnectAttempt = 0;
  const unsigned long RECONNECT_INTERVAL = 10000; // 10 segundos
  uint32_t frameCounter = 0;

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

      if (action == "toggle_yolo")
      {
        bool enabled = doc.containsKey("enabled") ? doc["enabled"].as<bool>() : !yoloController.isEnabled();
        yoloController.setEnabled(enabled);
        Serial.printf("[MQTT] YOLO %s via comando remoto\n", enabled ? "ativado" : "desativado");
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

WiFiConnector wifiConnector;
CameraController cameraController;
YoloController yoloController;
MQTTPublisher mqttPublisher;

static void startCameraServer();

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
    <h1>ESP32-CAM Smart Vision</h1>
    <p class="subtitle">Monitore o streaming e ative/desative o YOLO com apenas um clique.</p>
    <div class="video-box">
      <img id="stream" src="/stream" alt="Live stream" />
    </div>
    <div class="controls">
      <button id="toggleBtn">Carregando...</button>
      <span id="yoloBadge" class="badge off">YOLO: Desativado</span>
    </div>
    <div class="info-card">
      <strong>Endpoint YOLO:</strong>
      <span id="yoloEndpoint">--</span>
      <br />
      <small>Integre este firmware a um servidor YOLO externo para inferência completa.</small>
    </div>
  </div>
  <script>
    let yoloState = false;
    async function fetchYoloState() {
      try {
        const res = await fetch('/api/yolo');
        const data = await res.json();
        yoloState = !!data.enabled;
        const endpoint = (data.endpoint || '').length ? data.endpoint : 'não configurado';
        document.getElementById('yoloEndpoint').textContent = endpoint;
        updateUI();
      } catch (err) {
        console.error('Falha ao obter estado do YOLO', err);
      }
    }
    async function toggleYolo() {
      try {
        const target = !yoloState;
        await fetch(`/api/yolo/toggle?enabled=${target}`, { method: 'POST' });
        await fetchYoloState();
      } catch (err) {
        alert('Não foi possível alterar o estado do YOLO.');
      }
    }
    function updateUI() {
      const btn = document.getElementById('toggleBtn');
      const badge = document.getElementById('yoloBadge');
      if (yoloState) {
        btn.textContent = 'Desativar YOLO';
        badge.textContent = 'YOLO: Ativado';
        badge.classList.add('on');
        badge.classList.remove('off');
      } else {
        btn.textContent = 'Ativar YOLO';
        badge.textContent = 'YOLO: Desativado';
        badge.classList.add('off');
        badge.classList.remove('on');
      }
    }
    document.getElementById('toggleBtn').addEventListener('click', toggleYolo);
    window.addEventListener('load', () => {
      fetchYoloState();
      setInterval(fetchYoloState, 7000);
    });
  </script>
</body>
</html>
)rawliteral";

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

void loop()
{
  // Manter conexão MQTT ativa e processar mensagens
  mqttPublisher.loop();
  
  delay(10); // Pequeno delay para não sobrecarregar
}

/********** Servidor da câmera baseado no exemplo oficial **********/

static esp_err_t root_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t yolo_status_handler(httpd_req_t *req)
{
  String json = "{\"enabled\":";
  json += yoloController.isEnabled() ? "true" : "false";
  json += ",\"endpoint\":\"";
  json += yoloController.getEndpoint();
  json += "\"}";
  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, json.c_str(), json.length());
}

static esp_err_t yolo_toggle_handler(httpd_req_t *req)
{
  const size_t query_len = httpd_req_get_url_query_len(req) + 1;
  bool desired = !yoloController.isEnabled();

  if (query_len > 1 && query_len < 64)
  {
    char query[64];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
      char value[16];
      if (httpd_query_key_value(query, "enabled", value, sizeof(value)) == ESP_OK)
      {
        if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0)
        {
          desired = true;
        }
        else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0)
        {
          desired = false;
        }
      }
    }
  }

  yoloController.setEnabled(desired);
  return yolo_status_handler(req);
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
      // Processar YOLO (rápido, não bloqueante)
      yoloController.processFrame(fb);
      
      // Enviar frame via HTTP stream PRIMEIRO (prioridade)
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

  httpd_uri_t yolo_state_uri = {
      .uri = "/api/yolo",
      .method = HTTP_GET,
      .handler = yolo_status_handler,
      .user_ctx = nullptr};

  httpd_uri_t yolo_toggle_uri = {
      .uri = "/api/yolo/toggle",
      .method = HTTP_POST,
      .handler = yolo_toggle_handler,
      .user_ctx = nullptr};

  httpd_handle_t stream_httpd = nullptr;

  if (httpd_start(&stream_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(stream_httpd, &root_uri);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    httpd_register_uri_handler(stream_httpd, &status_uri);
    httpd_register_uri_handler(stream_httpd, &yolo_state_uri);
    httpd_register_uri_handler(stream_httpd, &yolo_toggle_uri);
    Serial.println("Rotas ativas:");
    Serial.println("  /stream  -> MJPEG ao vivo");
    Serial.println("  /status  -> informações da câmera");
    Serial.println("  /api/yolo -> estado do YOLO");
    Serial.println("  /api/yolo/toggle -> ativa/desativa YOLO");
    Serial.println("  /        -> painel web interativo");
  }
  else
  {
    Serial.println("Falha ao iniciar servidor HTTP");
  }
}

