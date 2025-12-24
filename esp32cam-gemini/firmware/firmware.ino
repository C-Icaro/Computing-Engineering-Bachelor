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

// Deep Sleep
#include "esp_sleep.h"

// ============================================================================
// ==== CONFIGURAÇÕES DE HARDWARE - CONSTANTES ====
// ============================================================================

// Pinagem dos sensores de entrada
#define PIR_PIN 12                    // GPIO 12: Sensor PIR (detecção de movimento)
#define BUTTON_PIN 13                 // GPIO 13: Botão físico (pull-up externo)

// Pinagem dos LEDs de saída
#define LED_RED_PIN 14                // GPIO 14: LED vermelho (pessoa detectada)
#define LED_GREEN_PIN 15              // GPIO 15: LED verde (sem pessoa)
#define FLASH_LED_PIN 4               // GPIO 4: Flash LED da câmera

// Configurações do ADC para leitura de bateria
#define BATTERY_ADC_PIN 33            // GPIO 33: Entrada ADC para leitura de bateria
#define DIVIDER_RATIO 0.689           // Razão do divisor de tensão: Rbot/(Rtop+Rbot) = 22.2/32.2
#define ADC_RESOLUTION 4095           // Resolução do ADC: 12 bits (0-4095)
#define ADC_MAX_VOLTAGE 3.3           // Tensão máxima do ADC: 3.3V (atenuação padrão)

// Constantes para cálculo de porcentagem de bateria de lítio (1 célula)
#define BATTERY_VOLTAGE_MAX 4.2       // Volts - 100% de carga
#define BATTERY_VOLTAGE_MIN 3.0       // Volts - 0% de carga (descarga profunda)
#define BATTERY_VOLTAGE_NOMINAL 3.7   // Volts - tensão nominal (~50%)

// Constantes de timing e delays
#define FLASH_STABILIZE_DELAY 100     // ms - Delay para estabilização do flash LED
#define DEEP_SLEEP_DELAY 2000         // ms - Delay antes de entrar em deep sleep
#define SERIAL_BAUD_RATE 115200       // Baud rate do Serial Monitor
#define SERIAL_INIT_DELAY 1000        // ms - Delay inicial do Serial
#define PIR_STABILIZE_DELAY 200       // ms - Delay para estabilização do PIR
#define PIR_CHECK_DELAY 100           // ms - Delay entre leituras do PIR
#define BUTTON_STABILIZE_DELAY 50     // ms - Delay para estabilização do botão
#define PIR_WAKE_STABILIZE_DELAY 2000 // ms - Delay após acordar pelo PIR
#define BUTTON_WAKE_STABILIZE_DELAY 500 // ms - Delay após acordar pelo botão
#define LOOP_DELAY 10                 // ms - Delay no loop principal
#define BATTERY_READ_INTERVAL 5000    // ms - Intervalo entre leituras de bateria
#define PIR_STABILITY_CHECK_COUNT 10  // Número de verificações de estabilidade do PIR
#define PIR_STABILITY_CHECK_DELAY 100 // ms - Delay entre verificações de estabilidade

// Constantes de debounce e controle de sensores
#define BUTTON_DEBOUNCE_DELAY 50      // ms - Delay de debounce do botão
#define PIR_DEBOUNCE_DELAY 500        // ms - Delay de debounce do PIR
#define PIR_COOLDOWN_PERIOD 10000     // ms - Período de cooldown entre detecções do PIR
#define PIR_STABILITY_READINGS 5      // Número de leituras consecutivas para confirmar detecção

// Constantes de comunicação de rede
#define WIFI_CONNECT_TIMEOUT 20000     // ms - Timeout para conexão WiFi
#define WIFI_CONNECT_RETRY_DELAY 500  // ms - Delay entre tentativas de conexão WiFi
#define HTTP_RESPONSE_TIMEOUT 15000    // ms - Timeout para resposta HTTP
#define HTTP_RESPONSE_READ_TIMEOUT 5000 // ms - Timeout para leitura completa da resposta
#define HTTP_RESPONSE_MAX_LENGTH 2000 // bytes - Tamanho máximo da resposta HTTP
#define GEMINI_REQUEST_TIMEOUT 10000  // ms - Timeout para requisição ao Gemini
#define HTTPS_PORT 443                 // Porta HTTPS padrão
#define HTTP_PORT 80                   // Porta HTTP padrão

// ==== Configurações de rede ====

// TODO: substitua pelas credenciais reais de Wi-Fi
const char* WIFI_SSID     = "Server";
const char* WIFI_PASSWORD = "server123";

// ==== Configurações da API Gemini ====

// TODO: substitua pela sua API key do Gemini
const char* GEMINI_API_KEY = "sua_api_gemini";

// Host e caminho do modelo Gemini 2.5 Flash-Lite
const char* GEMINI_HOST = "generativelanguage.googleapis.com";
const char* GEMINI_MODEL_PATH = "/v1beta/models/gemini-2.5-flash-lite:generateContent";

// ==== Configurações da Plataforma Web (Vercel) ====
// TODO: substitua pela URL da sua aplicação deployada na Vercel
const char* WEB_APP_HOST = "seu-app.vercel.app";  // Apenas o hostname, sem https:// e sem /
const char* WEB_APP_PATH = "/api/upload";
const uint16_t WEB_APP_PORT = 443;  // HTTPS

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

/**
 * Lê o nível de bateria através do divisor de tensão no GPIO 33
 * 
 * @return float Tensão da bateria em Volts. Retorna 0.0 em caso de erro.
 * 
 * @note Utiliza divisor de tensão com Rtop=10kΩ e Rbot=22.2kΩ
 * @note ADC configurado para 12 bits (0-4095) com atenuação padrão (0-3.3V)
 */
float readBatteryVoltage() {
  // Validação: verificar se o pino ADC está configurado
  if (BATTERY_ADC_PIN < 0) {
    Serial.println("ERRO: Pino ADC não configurado");
    return 0.0;
  }
  
  // Ler valor ADC (0-4095)
  int adcValue = analogRead(BATTERY_ADC_PIN);
  
  // Validação: verificar se a leitura é válida
  if (adcValue < 0 || adcValue > ADC_RESOLUTION) {
    Serial.print("ERRO: Leitura ADC inválida: ");
    Serial.println(adcValue);
    return 0.0;
  }
  
  // Converter para tensão no pino ADC (0-3.3V com atenuação padrão)
  float adcVoltage = (float)adcValue / ADC_RESOLUTION * ADC_MAX_VOLTAGE;
  
  // Validação: verificar se a tensão calculada é razoável
  if (adcVoltage < 0.0 || adcVoltage > ADC_MAX_VOLTAGE) {
    Serial.print("ERRO: Tensão ADC calculada inválida: ");
    Serial.print(adcVoltage);
    Serial.println("V");
    return 0.0;
  }
  
  // Calcular tensão real da bateria através do divisor de tensão
  // V_bateria = V_adc / DIVIDER_RATIO
  float batteryVoltage = adcVoltage / DIVIDER_RATIO;
  
  // Validação: verificar se a tensão da bateria está em faixa razoável
  if (batteryVoltage < 0.0 || batteryVoltage > 5.0) {
    Serial.print("AVISO: Tensão de bateria fora da faixa esperada: ");
    Serial.print(batteryVoltage);
    Serial.println("V");
  }
  
  return batteryVoltage;
}

/**
 * Calcula a porcentagem de carga da bateria de lítio baseada na tensão
 * 
 * @param voltage Tensão da bateria em Volts
 * @return int Porcentagem de carga (0-100). Retorna -1 em caso de erro.
 * 
 * @note Usa curva não-linear que reflete o comportamento real de células de lítio
 * @note A maior parte da capacidade está entre 3.7V e 4.2V
 */
int calculateBatteryPercentage(float voltage) {
  // Validação: verificar se a tensão é válida
  if (voltage < 0.0 || voltage > 5.0) {
    Serial.print("ERRO: Tensão inválida para cálculo de porcentagem: ");
    Serial.print(voltage);
    Serial.println("V");
    return -1;
  }
  
  // Limitar valores extremos
  if (voltage >= BATTERY_VOLTAGE_MAX) {
    return 100;
  }
  if (voltage <= BATTERY_VOLTAGE_MIN) {
    return 0;
  }
  
  // Curva de descarga aproximada para células de lítio
  // A curva não é linear - a maior parte da capacidade está entre 3.7V e 4.2V
  float percentage;
  
  if (voltage >= 3.9) {
    // Região alta (3.9V - 4.2V): ~80% - 100%
    // Mais linear nesta região
    percentage = 80.0 + ((voltage - 3.9) / (BATTERY_VOLTAGE_MAX - 3.9)) * 20.0;
  } else if (voltage >= 3.7) {
    // Região média-alta (3.7V - 3.9V): ~50% - 80%
    percentage = 50.0 + ((voltage - 3.7) / (3.9 - 3.7)) * 30.0;
  } else if (voltage >= 3.5) {
    // Região média-baixa (3.5V - 3.7V): ~20% - 50%
    percentage = 20.0 + ((voltage - 3.5) / (3.7 - 3.5)) * 30.0;
  } else {
    // Região baixa (3.0V - 3.5V): 0% - 20%
    percentage = ((voltage - BATTERY_VOLTAGE_MIN) / (3.5 - BATTERY_VOLTAGE_MIN)) * 20.0;
  }
  
  // Garantir que está entre 0 e 100
  if (percentage > 100.0) percentage = 100.0;
  if (percentage < 0.0) percentage = 0.0;
  
  return (int)percentage;
}

// Imprime informações da bateria no Serial Monitor
void printBatteryStatus() {
  int adcValue = analogRead(BATTERY_ADC_PIN);
  float adcVoltage = (float)adcValue / ADC_RESOLUTION * ADC_MAX_VOLTAGE;
  float batteryVoltage = readBatteryVoltage();
  int batteryPercentage = calculateBatteryPercentage(batteryVoltage);
  
  Serial.println("=== Status da Bateria ===");
  Serial.print("ADC Raw: ");
  Serial.println(adcValue);
  Serial.print("Tensão no ADC: ");
  Serial.print(adcVoltage, 3);
  Serial.println(" V");
  Serial.print("Tensão da Bateria: ");
  Serial.print(batteryVoltage, 3);
  Serial.print(" V");
  
  // Indicador visual de porcentagem
  Serial.print(" | Carga: ");
  Serial.print(batteryPercentage);
  Serial.print("%");
  
  // Barra de progresso simples
  Serial.print(" [");
  int bars = batteryPercentage / 5; // 20 barras = 100%
  for (int i = 0; i < 20; i++) {
    if (i < bars) {
      Serial.print("█");
    } else {
      Serial.print("░");
    }
  }
  Serial.print("]");
  
  // Indicador de status textual
  if (batteryPercentage >= 80) {
    Serial.print(" (Alta)");
  } else if (batteryPercentage >= 50) {
    Serial.print(" (Média)");
  } else if (batteryPercentage >= 20) {
    Serial.print(" (Baixa)");
  } else {
    Serial.print(" (CRÍTICA!)");
  }
  
  Serial.println();
  Serial.println("==========================");
}

/**
 * Conecta ao Wi-Fi com tratamento de erros e timeout
 * 
 * @return bool true se conectado com sucesso, false caso contrário
 * 
 * @note Utiliza timeout configurável via constante WIFI_CONNECT_TIMEOUT
 * @note Valida credenciais antes de tentar conexão
 */
bool connectWiFi() {
  // Validação: verificar se as credenciais foram configuradas
  if (strlen(WIFI_SSID) == 0) {
    Serial.println("ERRO: SSID do Wi-Fi não configurado");
    return false;
  }
  
  Serial.println("Conectando ao Wi-Fi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  
  // Configurar modo estação (cliente)
  WiFi.mode(WIFI_STA);
  
  // Tentar conectar
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Aguardar conexão com timeout
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && 
         (millis() - startAttemptTime) < WIFI_CONNECT_TIMEOUT) {
    delay(WIFI_CONNECT_RETRY_DELAY);
    Serial.print(".");
  }

  // Verificar resultado da conexão
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("✓ Wi-Fi conectado. IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    return true;
  } else {
    Serial.println("");
    Serial.println("✗ ERRO: Falha ao conectar ao Wi-Fi");
    Serial.print("Status: ");
    Serial.println(WiFi.status());
    return false;
  }
}

/**
 * Captura uma imagem da câmera com tratamento de erros
 * 
 * @return camera_fb_t* Ponteiro para o frame buffer. Retorna nullptr em caso de erro.
 * 
 * @note Liga o flash LED antes da captura para melhor iluminação
 * @note O frame buffer deve ser liberado com esp_camera_fb_return() após uso
 * @note Valida se a câmera foi inicializada antes de capturar
 */
camera_fb_t* captureImage() {
  Serial.println("Capturando imagem...");
  
  // Validação: verificar se a câmera foi inicializada
  // (não há função direta para isso, mas podemos tentar capturar)
  
  // Ligar o flash LED antes de capturar para melhor iluminação
  digitalWrite(FLASH_LED_PIN, HIGH);
  delay(FLASH_STABILIZE_DELAY); // Delay para o flash estabilizar
  
  // Capturar frame da câmera
  camera_fb_t* fb = esp_camera_fb_get();
  
  // Desligar o flash LED após capturar
  digitalWrite(FLASH_LED_PIN, LOW);
  
  // Validação: verificar se a captura foi bem-sucedida
  if (!fb) {
    Serial.println("✗ ERRO: Falha na captura da câmera");
    Serial.println("Verifique se a câmera foi inicializada corretamente");
    return nullptr;
  }
  
  // Validação: verificar se o frame tem dados válidos
  if (fb->len == 0) {
    Serial.println("✗ ERRO: Frame capturado está vazio");
    esp_camera_fb_return(fb);
    return nullptr;
  }
  
  // Validação: verificar se o tamanho do frame é razoável
  if (fb->len > 1000000) { // 1MB é um limite razoável para JPEG
    Serial.print("AVISO: Frame muito grande: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }

  Serial.print("✓ Imagem capturada. Tamanho: ");
  Serial.print(fb->len);
  Serial.println(" bytes");

  return fb;
}

// Envia a imagem para o Gemini 2.5 Flash-Lite usando JSON com inline_data base64
// Retorna true se sucesso, e personDetected será preenchido com a decisão
bool sendImageToGemini(camera_fb_t* fb, bool* personDetected) {
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

  if (!client.connect(GEMINI_HOST, HTTPS_PORT)) {
    Serial.print("✗ ERRO: Falha ao conectar ao host Gemini (");
    Serial.print(GEMINI_HOST);
    Serial.print(":");
    Serial.print(HTTPS_PORT);
    Serial.println(")");
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

  // Ler resposta com timeout configurável
  unsigned long timeout = millis();
  while (client.connected() && !client.available()) {
    if (millis() - timeout > GEMINI_REQUEST_TIMEOUT) {
      Serial.println("✗ ERRO: Timeout aguardando resposta do Gemini");
      client.stop();
      return false;
    }
    delay(LOOP_DELAY);
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
  bool detected = false;
  if (response.indexOf("no_person") != -1) {
    detected = false;
  } else if (response.indexOf("person") != -1) {
    detected = true;
  }

  // Retornar a decisão através do parâmetro
  if (personDetected != nullptr) {
    *personDetected = detected;
  }

  if (detected) {
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
      lastDecision = detected ? "person" : "no_person";
      Serial.println("Última imagem e decisão armazenadas para visualização web.");
    } else {
      Serial.println("Falha ao alocar memória para armazenar última imagem.");
    }

  client.stop();
  return true;
}

// Envia a imagem e decisão para a plataforma web (Vercel)
bool sendImageToWebApp(camera_fb_t* fb, bool personDetected) {
  if (!fb) {
    Serial.println("Frame buffer nulo, não é possível enviar para a plataforma web");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado, tentando reconectar...");
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Ainda sem Wi-Fi. Abortando envio para plataforma web.");
      return false;
    }
  }

  WiFiClientSecure client;
  client.setInsecure(); // Para MVP: desabilita verificação de certificado

  Serial.print("Conectando a ");
  Serial.print(WEB_APP_HOST);
  Serial.println("...");

  if (!client.connect(WEB_APP_HOST, WEB_APP_PORT)) {
    Serial.print("✗ ERRO: Falha ao conectar ao servidor web (");
    Serial.print(WEB_APP_HOST);
    Serial.print(":");
    Serial.print(WEB_APP_PORT);
    Serial.println(")");
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

  // Ler dados da bateria antes de enviar
  float batteryVoltage = readBatteryVoltage();
  int batteryPercentage = calculateBatteryPercentage(batteryVoltage);
  
  Serial.print("Dados de bateria: ");
  Serial.print(batteryVoltage, 3);
  Serial.print("V, ");
  Serial.print(batteryPercentage);
  Serial.println("%");
  
  // Montar o payload JSON
  String decision = personDetected ? "person" : "no_person";
  String payload = "{";
  payload += "\"image\":\"data:image/jpeg;base64," + base64Data + "\",";
  payload += "\"decision\":\"" + decision + "\",";
  payload += "\"battery\":{";
  payload += "\"voltage\":" + String(batteryVoltage, 3) + ",";
  payload += "\"percentage\":" + String(batteryPercentage);
  payload += "}";
  payload += "}";

  Serial.print("Tamanho do payload: ");
  Serial.print(payload.length());
  Serial.println(" bytes");

  // Cabeçalhos HTTP
  String request = String("POST ") + String(WEB_APP_PATH) + " HTTP/1.1\r\n" +
                   "Host: " + String(WEB_APP_HOST) + "\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + String(payload.length()) + "\r\n" +
                   "Connection: close\r\n\r\n";

  Serial.println("Enviando imagem para plataforma web...");
  Serial.print("Host: ");
  Serial.println(WEB_APP_HOST);
  Serial.print("Path: ");
  Serial.println(WEB_APP_PATH);
  
  // Enviar requisição
  size_t requestLen = client.print(request);
  size_t payloadLen = client.print(payload);
  
  Serial.print("Bytes enviados - Request: ");
  Serial.print(requestLen);
  Serial.print(", Payload: ");
  Serial.println(payloadLen);

  // Aguardar resposta com timeout configurável
  unsigned long timeout = millis();
  while (client.connected() && !client.available()) {
    if (millis() - timeout > HTTP_RESPONSE_TIMEOUT) {
      Serial.print("✗ ERRO: Timeout aguardando resposta da plataforma web (");
      Serial.print(HTTP_RESPONSE_TIMEOUT / 1000);
      Serial.println("s)");
      client.stop();
      return false;
    }
    delay(LOOP_DELAY);
  }

  // Ler resposta completa
  String response = "";
  String statusLine = "";
  int httpCode = 0;
  bool firstLine = true;
  
  // Ler resposta completa com timeout e limite de tamanho
  unsigned long responseTimeout = millis();
  while (client.available() || client.connected()) {
    if (millis() - responseTimeout > HTTP_RESPONSE_READ_TIMEOUT) {
      Serial.println("AVISO: Timeout lendo resposta completa");
      break;
    }
    
    if (client.available()) {
      String line = client.readStringUntil('\n');
      response += line + "\n";
      
      if (firstLine) {
        statusLine = line;
        // Extrair código HTTP (ex: "HTTP/1.1 200 OK")
        int spaceIndex = line.indexOf(' ');
        if (spaceIndex > 0) {
          int codeIndex = spaceIndex + 1;
          int codeEndIndex = line.indexOf(' ', codeIndex);
          if (codeEndIndex > 0) {
            httpCode = line.substring(codeIndex, codeEndIndex).toInt();
          }
        }
        firstLine = false;
      }
      
      // Limitar tamanho da resposta para não sobrecarregar memória
      if (response.length() > HTTP_RESPONSE_MAX_LENGTH) {
        response += "... (truncado)";
        Serial.print("AVISO: Resposta truncada em ");
        Serial.print(HTTP_RESPONSE_MAX_LENGTH);
        Serial.println(" bytes");
        break;
      }
    } else {
      delay(LOOP_DELAY);
    }
  }

  Serial.println("=== Resposta da plataforma web ===");
  Serial.print("Status: ");
  Serial.println(statusLine);
  Serial.print("Código HTTP: ");
  Serial.println(httpCode);
  Serial.println("Resposta completa:");
  Serial.println(response);
  Serial.println("===================================");

  client.stop();
  
  // Verificar se foi sucesso
  if (httpCode >= 200 && httpCode < 300) {
    Serial.println("✓ Envio bem-sucedido!");
    return true;
  } else {
    Serial.print("✗ Erro no envio. Código HTTP: ");
    Serial.println(httpCode);
    return false;
  }
}

// ============================================================================
// ==== ESTRUTURAS DE DADOS PARA ENCAPSULAMENTO ====
// ============================================================================

/**
 * Estrutura para encapsular estado do botão e reduzir variáveis globais
 */
struct ButtonState {
  int currentState;           // Estado atual do botão (HIGH/LOW)
  int lastReading;           // Última leitura do botão
  unsigned long lastDebounceTime; // Timestamp da última mudança para debounce
};

/**
 * Estrutura para encapsular estado do sensor PIR e reduzir variáveis globais
 */
struct PIRState {
  int currentState;              // Estado atual do PIR (HIGH/LOW)
  int lastReading;                // Última leitura do PIR
  unsigned long lastDebounceTime; // Timestamp da última mudança para debounce
  unsigned long lastTriggerTime;  // Timestamp da última detecção confirmada
  int stabilityCount;             // Contador de leituras consecutivas para estabilidade
  bool isConnected;               // Flag indicando se o PIR está conectado
};

// ============================================================================
// ==== VARIÁVEIS GLOBAIS (Mínimas necessárias) ====
// ============================================================================

// Instâncias das estruturas de estado (encapsulamento)
ButtonState buttonState = {HIGH, HIGH, 0};
PIRState pirState = {LOW, LOW, 0, 0, 0, false};

// Buffer da última imagem capturada (necessário para servidor web interno)
uint8_t* lastImage = nullptr;
size_t lastImageLen = 0;
String lastDecision = "unknown";

// Função para processar captura e envio (usada tanto por botão quanto PIR)
void processCaptureAndSend() {
  Serial.println("Iniciando captura e envio ao Gemini...");

  camera_fb_t* fb = captureImage();
  if (fb) {
    bool personDetected = false;
    bool ok = sendImageToGemini(fb, &personDetected);
    if (ok) {
      Serial.println("Envio ao Gemini concluído.");
    } else {
      Serial.println("Falha no envio ao Gemini. Continuando mesmo assim...");
      // Continuar mesmo se o Gemini falhar
    }
    
    // Sempre tentar enviar para a plataforma web, mesmo se o Gemini falhar
    Serial.println("Enviando imagem para plataforma web...");
    bool webOk = sendImageToWebApp(fb, personDetected);
    if (webOk) {
      Serial.println("Envio para plataforma web concluído.");
    } else {
      Serial.println("Falha no envio para plataforma web.");
    }

    // Liberar frame buffer para evitar vazamento de memória
    esp_camera_fb_return(fb);
  }

  // Aguardar um pouco antes de entrar em deep sleep (permite logs finais)
  delay(DEEP_SLEEP_DELAY);
  
  // Entrar em deep sleep
  Serial.println("Entrando em modo deep sleep...");
  Serial.println("PIR (GPIO 12) e Botão (GPIO 13) permanecem ativos e acordarão o sistema.");
  Serial.println("Ambos acionarão captura e envio ao Gemini automaticamente ao acordar.");
  
  // Configurar wake-up sources usando EXT1 para permitir múltiplos pinos
  // PIR (GPIO 12): acorda quando detecta movimento (HIGH) - apenas se estiver conectado
  // Botão (GPIO 13): será verificado manualmente no setup (LOW quando pressionado)
  // Usamos ANY_HIGH para detectar quando o PIR está HIGH
  if (pirState.isConnected) {
    uint64_t wakeup_pin_mask = ((uint64_t)1 << PIR_PIN);
    esp_err_t err = esp_sleep_enable_ext1_wakeup(wakeup_pin_mask, ESP_EXT1_WAKEUP_ANY_HIGH);
    if (err == ESP_OK) {
      Serial.println("✓ Wake-up do PIR habilitado para deep sleep");
    } else {
      Serial.print("✗ ERRO ao habilitar wake-up do PIR: ");
      Serial.println(err);
    }
  } else {
    Serial.println("AVISO: Wake-up do PIR desabilitado - sensor pode estar desconectado");
  }
  
  // Nota: O botão (GPIO 13) é LOW quando pressionado, então será verificado manualmente
  // no setup após acordar, já que EXT1 não pode detectar LOW diretamente com ANY_HIGH
  // Quando detectado, também processará captura automaticamente
  
  // Entrar em deep sleep
  esp_deep_sleep_start();
}

void setup() {

  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Inicializar Serial Monitor com baud rate configurável
  Serial.begin(SERIAL_BAUD_RATE);
  delay(SERIAL_INIT_DELAY);

  // Verificar se acordou do deep sleep
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  // Verificar qual GPIO acordou o sistema
  bool wokeByPIR = false;
  bool wokeByButton = false;
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    // Acordou pelo EXT0 (PIR no GPIO 12)
    wokeByPIR = true;
    Serial.println("Acordou pelo sensor PIR (GPIO 12) - movimento detectado!");
  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
    // Verificar qual pino do EXT1 acordou
    uint64_t wakeup_pin = esp_sleep_get_ext1_wakeup_status();
    if (wakeup_pin & ((uint64_t)1 << PIR_PIN)) {
      wokeByPIR = true;
      Serial.println("Acordou pelo sensor PIR (GPIO 12) - movimento detectado!");
    }
    if (wakeup_pin & ((uint64_t)1 << BUTTON_PIN)) {
      wokeByButton = true;
      Serial.println("Acordou pelo botão (GPIO 13)!");
    }
  } else {
    Serial.println("Inicialização normal (não foi deep sleep)");
  }

  // Inicializa sensor PIR
  // O PIR deve estar configurado como RTC GPIO para funcionar no deep sleep
  pinMode(PIR_PIN, INPUT);
  rtc_gpio_init((gpio_num_t)PIR_PIN);
  rtc_gpio_set_direction((gpio_num_t)PIR_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pulldown_en((gpio_num_t)PIR_PIN);  // Habilita pull-down para evitar leituras flutuantes quando desconectado
  rtc_gpio_pullup_dis((gpio_num_t)PIR_PIN);
  
  // Verificar se o PIR está conectado
  // Se o pino estiver HIGH imediatamente após inicialização com pull-down, pode ser ruído
  // Aguardar um pouco e verificar novamente
  delay(PIR_STABILIZE_DELAY);
  int pirCheck1 = digitalRead(PIR_PIN);
  delay(PIR_CHECK_DELAY);
  int pirCheck2 = digitalRead(PIR_PIN);
  delay(PIR_CHECK_DELAY);
  int pirCheck3 = digitalRead(PIR_PIN);
  
  // Se todas as leituras forem LOW, o PIR provavelmente está desconectado ou não detectando movimento
  // Se alguma leitura for HIGH, pode ser que o PIR esteja conectado e detectando movimento
  // Mas se estiver HIGH constantemente sem movimento, pode ser que esteja desconectado
  if (pirCheck1 == LOW && pirCheck2 == LOW && pirCheck3 == LOW) {
    pirState.isConnected = true; // PIR provavelmente conectado (em estado LOW normal)
    Serial.println("✓ PIR inicializado (pull-down habilitado para evitar leituras flutuantes)");
  } else {
    // Se estiver HIGH, pode ser movimento real ou pino flutuante
    // Vamos assumir que está conectado, mas avisar
    pirState.isConnected = true;
    Serial.println("AVISO: PIR detectou sinal HIGH na inicialização. Verifique se o sensor está conectado corretamente.");
  }

  // Inicializa botão (pull-up externo conforme esquema do DroneBot)
  // O botão também é RTC GPIO e pode acordar do deep sleep
  pinMode(BUTTON_PIN, INPUT);
  rtc_gpio_init((gpio_num_t)BUTTON_PIN);
  rtc_gpio_set_direction((gpio_num_t)BUTTON_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pulldown_dis((gpio_num_t)BUTTON_PIN);
  rtc_gpio_pullup_dis((gpio_num_t)BUTTON_PIN); // Pull-up externo conforme esquema

  // Verificar manualmente o botão após inicializar os pinos
  // Isso é necessário porque o botão é LOW quando pressionado e pode não ser detectado pelo EXT1
  delay(BUTTON_STABILIZE_DELAY); // Pequeno delay para estabilização
  if (digitalRead(BUTTON_PIN) == LOW && !wokeByPIR && wakeup_reason != ESP_SLEEP_WAKEUP_UNDEFINED) {
    wokeByButton = true;
    Serial.println("✓ Botão detectado pressionado ao acordar!");
  }

  // Inicializa LEDs de indicação
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);

  // Inicializa flash LED da câmera
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  // Inicializa ADC para leitura de bateria (GPIO 33)
  // Configurar resolução para 12 bits (0-4095)
  // A atenuação padrão do GPIO 33 já permite ler até 3.3V (11dB)
  analogReadResolution(12); // 12 bits (0-4095)
  Serial.println("ADC de bateria configurado (GPIO 33)");
  
  // Fazer uma leitura inicial e imprimir status da bateria
  delay(BUTTON_STABILIZE_DELAY * 2); // Delay para estabilização do ADC
  printBatteryStatus();

  // Initialize the camera
  Serial.print("Initializing the camera module...");
  configESPCamera();
  Serial.println("Camera OK!");

  // Conecta ao Wi-Fi
  connectWiFi();

  // Inicia servidor web interno na porta HTTP padrão
  webServer.begin();
  Serial.print("✓ Servidor web iniciado na porta ");
  Serial.print(HTTP_PORT);
  Serial.print(". Acesse: http://");
  Serial.println(WiFi.localIP());

  // Se acordou pelo PIR ou pelo botão, processar imediatamente
  if (wokeByPIR) {
    if (!pirState.isConnected) {
      Serial.println("AVISO: Sistema acordou pelo PIR, mas sensor pode estar desconectado. Ignorando...");
    } else {
      delay(PIR_WAKE_STABILIZE_DELAY); // Delay maior para estabilizar o PIR após acordar (reduz falsos positivos)
      // Verificar novamente se o PIR ainda está ativo após estabilização
      bool pirStillActive = false;
      for (int i = 0; i < PIR_STABILITY_CHECK_COUNT; i++) {
        if (digitalRead(PIR_PIN) == HIGH) {
          pirStillActive = true;
          break;
        }
        delay(PIR_STABILITY_CHECK_DELAY);
      }
      
      if (pirStillActive) {
        Serial.println("Processando captura acionada pelo PIR (confirmado após estabilização)...");
        pirState.lastTriggerTime = millis(); // Registrar tempo da detecção
        processCaptureAndSend();
      } else {
        Serial.println("PIR acionou mas não confirmou após estabilização - provável falso positivo, ignorando...");
      }
    }
  } else if (wokeByButton) {
    // Se acordou pelo botão, também processa automaticamente
    delay(BUTTON_WAKE_STABILIZE_DELAY); // Pequeno delay para estabilização após acordar
    Serial.println("Processando captura acionada pelo botão...");
    processCaptureAndSend();
  } else {
    Serial.println("Sistema pronto. Botão (GPIO 13) e PIR (GPIO 12) podem acordar do deep sleep e tirar foto automaticamente.");
  }

}

/**
 * Loop principal do programa
 * 
 * @note Processa leituras do botão e sensor PIR com debounce
 * @note Atende requisições do servidor web interno
 * @note Realiza leitura periódica da bateria
 */
void loop() {
  // --- Verificar botão com debounce ---
  int reading = digitalRead(BUTTON_PIN);

  // Detectar mudança de estado para iniciar debounce
  if (reading != buttonState.lastReading) {
    buttonState.lastDebounceTime = millis();
  }

  // Aplicar debounce: aguardar período de estabilidade
  if ((millis() - buttonState.lastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
    if (reading != buttonState.currentState) {
      buttonState.currentState = reading;

      // Botão pressionado (LOW quando pressionado, pull-up externo)
      if (buttonState.currentState == LOW) {
        Serial.println("✓ Botão pressionado!");
        processCaptureAndSend();
      }
    }
  }

  buttonState.lastReading = reading;

  // --- Verificar sensor PIR com debounce e cooldown ---
  // Apenas processar se o PIR estiver conectado
  if (!pirState.isConnected) {
    // PIR não conectado, pular processamento
    delay(LOOP_DELAY);
  } else {
    int pirReading = digitalRead(PIR_PIN);

    // Verificar se estamos em período de cooldown
    unsigned long timeSinceLastTrigger = millis() - pirState.lastTriggerTime;
    bool inCooldown = (timeSinceLastTrigger < PIR_COOLDOWN_PERIOD);
    
    if (!inCooldown) {
      // Apenas processar PIR se não estiver em cooldown
      
      // Detectar mudança de estado para iniciar debounce
      if (pirReading != pirState.lastReading) {
        pirState.lastDebounceTime = millis();
        pirState.stabilityCount = 0; // Resetar contador de estabilidade
      }

      // Aplicar debounce: aguardar período de estabilidade
      if ((millis() - pirState.lastDebounceTime) > PIR_DEBOUNCE_DELAY) {
        if (pirReading != pirState.currentState) {
          pirState.currentState = pirReading;

          // PIR detectou movimento (HIGH = movimento detectado)
          if (pirState.currentState == HIGH) {
            pirState.stabilityCount++;
            
            // Verificar estabilidade: precisa de múltiplas leituras consecutivas
            if (pirState.stabilityCount >= PIR_STABILITY_READINGS) {
              Serial.println("✓ Movimento detectado pelo PIR (confirmado após estabilidade)!");
              pirState.lastTriggerTime = millis(); // Registrar tempo da detecção
              pirState.stabilityCount = 0; // Resetar contador
              processCaptureAndSend();
            } else {
              // Ainda não confirmado, aguardar mais leituras
              Serial.print("PIR detectou movimento, aguardando confirmação (");
              Serial.print(pirState.stabilityCount);
              Serial.print("/");
              Serial.print(PIR_STABILITY_READINGS);
              Serial.println(")...");
            }
          } else {
            // PIR voltou para LOW, resetar contador
            pirState.stabilityCount = 0;
          }
        }
      }
    } else {
      // Em período de cooldown, apenas atualizar última leitura
      if (pirReading != pirState.lastReading) {
        Serial.print("PIR em cooldown (");
        Serial.print((PIR_COOLDOWN_PERIOD - timeSinceLastTrigger) / 1000);
        Serial.println("s restantes)...");
      }
    }

    pirState.lastReading = pirReading;
  } // Fim do if (pirState.isConnected)

  // Pequeno delay para reduzir ruído de leitura e economizar CPU
  delay(LOOP_DELAY);

  // --- Leitura periódica de bateria ---
  static unsigned long lastBatteryRead = 0;
  
  if (millis() - lastBatteryRead >= BATTERY_READ_INTERVAL) {
    lastBatteryRead = millis();
    printBatteryStatus();
  }

  // --- Servidor web interno: atender requisições HTTP ---
  WiFiClient webClient = webServer.available();
  if (webClient) {
    Serial.println("Cliente web conectado.");
    
    // Ler linha de requisição
    String requestLine = webClient.readStringUntil('\r');
    if (requestLine.length() == 0) {
      // Requisição inválida, fechar conexão
      webClient.stop();
      Serial.println("✗ Requisição HTTP inválida");
      return;
    }
    
    webClient.readStringUntil('\n'); // Descartar \n

    // Ler e descartar o restante dos cabeçalhos HTTP
    while (webClient.available()) {
      String headerLine = webClient.readStringUntil('\n');
      if (headerLine == "\r" || headerLine.length() == 1) {
        break; // Fim dos cabeçalhos
      }
    }

    // Verificar tipo de requisição
    bool requestImage = false;
    if (requestLine.startsWith("GET /image")) {
      requestImage = true;
    }

    // Responder conforme o tipo de requisição
    if (requestImage && lastImage != nullptr && lastImageLen > 0) {
      // Responder com a última imagem JPEG
      webClient.println("HTTP/1.1 200 OK");
      webClient.println("Content-Type: image/jpeg");
      webClient.print("Content-Length: ");
      webClient.println(lastImageLen);
      webClient.println("Connection: close");
      webClient.println();
      
      // Enviar dados da imagem
      size_t bytesWritten = webClient.write(lastImage, lastImageLen);
      if (bytesWritten != lastImageLen) {
        Serial.print("AVISO: Nem todos os bytes foram enviados. Esperado: ");
        Serial.print(lastImageLen);
        Serial.print(", Enviado: ");
        Serial.println(bytesWritten);
      }
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
        // Parâmetro ts para evitar cache do navegador
        webClient.print("<img src='/image.jpg?ts=");
        webClient.print(millis());
        webClient.println("' style='max-width:100%;height:auto;' />");
      } else {
        webClient.println("<p>Nenhuma imagem disponível ainda. Pressione o botão ou aguarde detecção do PIR.</p>");
      }

      webClient.println("</body></html>");
    }

    delay(1); // Pequeno delay para garantir que os dados foram enviados
    webClient.stop();
    Serial.println("Cliente web desconectado.");
  }

}