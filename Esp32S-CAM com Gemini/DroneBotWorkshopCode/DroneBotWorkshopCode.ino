/*
  ESP32 CAM Camera with MicroSD storage
  esp32cam-microsd.ino
  Take picture when button pressed
  Store image on MicroSD card

  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/
// Include Required Libraries

// Camera libraries
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"

// Wi-Fi & HTTPS
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Base64 (usado para enviar JPEG em JSON para o Gemini)
#include "mbedtls/base64.h"

// ==== Configurações de hardware ====

// Pino do botão físico (ligado conforme esquema com pull-up externo)
#define BUTTON_PIN 13

// LEDs de indicação de pessoa / não pessoa
// Conecte o LED vermelho (com resistor) ao GPIO 14 e o LED verde ao GPIO 15.
// Ambos com o outro terminal em GND.
#define LED_RED_PIN 14
#define LED_GREEN_PIN 15

// ==== Configurações de rede ====

// TODO: substitua pelas credenciais reais de Wi-Fi
const char* WIFI_SSID     = "Server";
const char* WIFI_PASSWORD = "server123";

// ==== Configurações da API Gemini ====

// TODO: substitua pela sua API key do Gemini
const char* GEMINI_API_KEY = "AIzaSyAvVt5kIAlf1OOCzzd_Svz1yjgLNSj2oAg";

// Host e caminho do modelo Gemini 2.5 Flash-Lite
const char* GEMINI_HOST = "generativelanguage.googleapis.com";
const char* GEMINI_MODEL_PATH = "/v1beta/models/gemini-2.5-flash-lite:generateContent";

// ==== Servidor web interno (HTTP) ====

WiFiServer webServer(80);

// Buffer da última imagem capturada (cópia do JPEG) e última decisão do Gemini
uint8_t* lastImage = nullptr;
size_t lastImageLen = 0;
String lastDecision = "unknown";

// Pin definitions for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void configESPCamera() {
  // Configure Camera parameters

  // Object to store the camera configuration parameters
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
  config.pixel_format = PIXFORMAT_JPEG; // Choices are YUV422, GRAYSCALE, RGB565, JPEG
  // Para evitar problemas de memória, usamos sempre um frame moderado
  config.frame_size = FRAMESIZE_SVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 12;           //10-63 lower número = maior qualidade
  config.fb_count = 1;

  // Initialize the Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Camera quality adjustments
  sensor_t * s = esp_camera_sensor_get();

  // BRIGHTNESS (-2 to 2)
  s->set_brightness(s, 0);
  // CONTRAST (-2 to 2)
  s->set_contrast(s, 0);
  // SATURATION (-2 to 2)
  s->set_saturation(s, 0);
  // SPECIAL EFFECTS (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_special_effect(s, 0);
  // WHITE BALANCE (0 = Disable , 1 = Enable)
  s->set_whitebal(s, 1);
  // AWB GAIN (0 = Disable , 1 = Enable)
  s->set_awb_gain(s, 1);
  // WB MODES (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_wb_mode(s, 0);
  // EXPOSURE CONTROLS (0 = Disable , 1 = Enable)
  s->set_exposure_ctrl(s, 1);
  // AEC2 (0 = Disable , 1 = Enable)
  s->set_aec2(s, 0);
  // AE LEVELS (-2 to 2)
  s->set_ae_level(s, 0);
  // AEC VALUES (0 to 1200)
  s->set_aec_value(s, 300);
  // GAIN CONTROLS (0 = Disable , 1 = Enable)
  s->set_gain_ctrl(s, 1);
  // AGC GAIN (0 to 30)
  s->set_agc_gain(s, 0);
  // GAIN CEILING (0 to 6)
  s->set_gainceiling(s, (gainceiling_t)0);
  // BPC (0 = Disable , 1 = Enable)
  s->set_bpc(s, 0);
  // WPC (0 = Disable , 1 = Enable)
  s->set_wpc(s, 1);
  // RAW GMA (0 = Disable , 1 = Enable)
  s->set_raw_gma(s, 1);
  // LENC (0 = Disable , 1 = Enable)
  s->set_lenc(s, 1);
  // HORIZ MIRROR (0 = Disable , 1 = Enable)
  s->set_hmirror(s, 0);
  // VERT FLIP (0 = Disable , 1 = Enable)
  s->set_vflip(s, 0);
  // DCW (0 = Disable , 1 = Enable)
  s->set_dcw(s, 1);
  // COLOR BAR PATTERN (0 = Disable , 1 = Enable)
  s->set_colorbar(s, 0);

}

// ==== Funções auxiliares ====

void connectWiFi() {
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 20000; // 20s

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Wi-Fi conectado. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Falha ao conectar ao Wi-Fi");
  }
}

// Captura uma imagem da câmera e retorna o frame buffer
camera_fb_t* captureImage() {
  Serial.println("Capturando imagem...");
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Falha na captura da câmera");
    return nullptr;
  }

  Serial.print("Imagem capturada. Tamanho (bytes): ");
  Serial.println(fb->len);

  return fb;
}

// Envia a imagem para o Gemini 2.5 Flash-Lite usando JSON com inline_data base64
bool sendImageToGemini(camera_fb_t* fb) {
  if (!fb) {
    Serial.println("Frame buffer nulo, não é possível enviar ao Gemini");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado, tentando reconectar...");
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Ainda sem Wi-Fi. Abortando envio ao Gemini.");
      return false;
    }
  }

  WiFiClientSecure client;
  client.setInsecure(); // Para MVP: desabilita verificação de certificado (não usar em produção)

  Serial.print("Conectando a ");
  Serial.print(GEMINI_HOST);
  Serial.println("...");

  if (!client.connect(GEMINI_HOST, 443)) {
    Serial.println("Falha ao conectar ao host Gemini");
    return false;
  }

  // Codificar o JPEG em base64
  size_t encoded_len = 0;
  size_t max_encoded_len = (fb->len * 4) / 3 + 4;
  unsigned char* encoded = (unsigned char*)malloc(max_encoded_len);
  if (!encoded) {
    Serial.println("Falha ao alocar memória para base64");
    return false;
  }

  int res = mbedtls_base64_encode(encoded, max_encoded_len, &encoded_len, fb->buf, fb->len);
  if (res != 0) {
    Serial.print("Erro ao codificar base64, código: ");
    Serial.println(res);
    free(encoded);
    return false;
  }

  String base64Data = String((char*)encoded);
  free(encoded);

  // Montar o payload JSON esperado pela API do Gemini
  // Instrução: responder apenas "person" ou "no_person"
  String payload = "{";
  payload += "\"contents\":[{\"parts\":[";
  payload += "{\"text\":\"Responda exatamente 'person' se houver pelo menos uma pessoa humana visível na imagem, ";
  payload += "ou 'no_person' se não houver nenhuma pessoa. Não explique, não adicione nada além dessas palavras.\"},";
  payload += "{\"inline_data\":{";
  payload += "\"mime_type\":\"image/jpeg\",";
  payload += "\"data\":\"" + base64Data + "\"";
  payload += "}}]}]";
  payload += "}";

  // Caminho incluindo a API key na query string
  String urlPath = String(GEMINI_MODEL_PATH) + "?key=" + GEMINI_API_KEY;

  // Cabeçalhos HTTP
  String request = String("POST ") + urlPath + " HTTP/1.1\r\n" +
                   "Host: " + String(GEMINI_HOST) + "\r\n" +
                   "Content-Type: application/json; charset=utf-8\r\n" +
                   "Content-Length: " + String(payload.length()) + "\r\n" +
                   "Connection: close\r\n\r\n";

  Serial.println("Enviando requisição ao Gemini...");
  client.print(request);
  client.print(payload);

  // Ler resposta
  unsigned long timeout = millis();
  while (client.connected() && !client.available()) {
    if (millis() - timeout > 10000) {
      Serial.println("Timeout aguardando resposta do Gemini");
      client.stop();
      return false;
    }
    delay(10);
  }

  // Ler resposta completa
  String response = "";
  while (client.available()) {
    String line = client.readStringUntil('\n');
    response += line + "\n";
  }

  Serial.println("Resposta do Gemini:");
  Serial.println(response);

  // Lógica simples: se contiver "no_person" => não há pessoa;
  // senão, se contiver "person" => há pessoa.
  bool personDetected = false;
  if (response.indexOf("no_person") != -1) {
    personDetected = false;
  } else if (response.indexOf("person") != -1) {
    personDetected = true;
  }

  if (personDetected) {
    Serial.println("Detecção: pessoa encontrada. Acendendo LED vermelho.");
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, LOW);
  } else {
    Serial.println("Detecção: nenhuma pessoa. Acendendo LED verde.");
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
  }

  // Atualizar cópia da última imagem e decisão para visualização via servidor web interno
  if (lastImage != nullptr) {
    free(lastImage);
    lastImage = nullptr;
    lastImageLen = 0;
  }

  lastImage = (uint8_t*)malloc(fb->len);
  if (lastImage) {
    memcpy(lastImage, fb->buf, fb->len);
    lastImageLen = fb->len;
    lastDecision = personDetected ? "person" : "no_person";
    Serial.println("Última imagem e decisão armazenadas para visualização web.");
  } else {
    Serial.println("Falha ao alocar memória para armazenar última imagem.");
  }

  client.stop();
  return true;
}

// ==== Variáveis do botão (debounce) ====

int buttonState = HIGH;
int lastButtonReading = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms

void setup() {

  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Start Serial Monitor
  Serial.begin(115200);
  delay(1000);

  // Inicializa botão (pull-up externo conforme esquema do DroneBot)
  pinMode(BUTTON_PIN, INPUT);

  // Inicializa LEDs de indicação
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);

  // Initialize the camera
  Serial.print("Initializing the camera module...");
  configESPCamera();
  Serial.println("Camera OK!");

  // Conecta ao Wi-Fi
  connectWiFi();

  // Inicia servidor web interno
  webServer.begin();
  Serial.print("Servidor web iniciado. Acesse: http://");
  Serial.println(WiFi.localIP());

}

void loop() {

  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      // Botão pressionado (considerando botão para GND com INPUT_PULLUP)
      if (buttonState == LOW) {
        Serial.println("Botão pressionado: iniciando captura e envio ao Gemini...");

        camera_fb_t* fb = captureImage();
        if (fb) {
          bool ok = sendImageToGemini(fb);
          if (ok) {
            Serial.println("Envio ao Gemini concluído.");
          } else {
            Serial.println("Falha no envio ao Gemini.");
          }

          // Liberar frame buffer para evitar vazamento de memória
          esp_camera_fb_return(fb);
        }
      }
    }
  }

  lastButtonReading = reading;

  // Pequeno delay para reduzir ruído de leitura
  delay(10);

  // --- Servidor web interno: atender requisições HTTP ---
  WiFiClient webClient = webServer.available();
  if (webClient) {
    Serial.println("Cliente web conectado.");
    String requestLine = webClient.readStringUntil('\r');
    webClient.readStringUntil('\n'); // descartar \n

    // Ler e descartar o restante dos cabeçalhos
    while (webClient.available()) {
      String headerLine = webClient.readStringUntil('\n');
      if (headerLine == "\r" || headerLine.length() == 1) {
        break;
      }
    }

    bool requestImage = false;
    if (requestLine.startsWith("GET /image")) {
      requestImage = true;
    }

    if (requestImage && lastImage != nullptr && lastImageLen > 0) {
      // Responder com a última imagem JPEG
      webClient.println("HTTP/1.1 200 OK");
      webClient.println("Content-Type: image/jpeg");
      webClient.print("Content-Length: ");
      webClient.println(lastImageLen);
      webClient.println("Connection: close");
      webClient.println();
      webClient.write(lastImage, lastImageLen);
    } else {
      // Página HTML simples com a decisão e a imagem (se houver)
      webClient.println("HTTP/1.1 200 OK");
      webClient.println("Content-Type: text/html; charset=utf-8");
      webClient.println("Connection: close");
      webClient.println();

      webClient.println("<!DOCTYPE html><html><head><meta charset='utf-8'><title>ESP32-CAM Gemini</title></head><body>");
      webClient.println("<h1>ESP32-CAM + Gemini</h1>");
      webClient.print("<p>Última decisão: <strong>");
      webClient.print(lastDecision);
      webClient.println("</strong></p>");

      if (lastImage != nullptr && lastImageLen > 0) {
        // parâmetro ts para evitar cache
        webClient.print("<img src='/image.jpg?ts=");
        webClient.print(millis());
        webClient.println("' style='max-width:100%;height:auto;' />");
      } else {
        webClient.println("<p>Nenhuma imagem disponível ainda. Pressione o botão para capturar.</p>");
      }

      webClient.println("</body></html>");
    }

    delay(1);
    webClient.stop();
    Serial.println("Cliente web desconectado.");
  }

}