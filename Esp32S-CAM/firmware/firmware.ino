/*
  esp32s-cam.ino
  Sistema de Monitoramento de Via Férrea
  - Sensor PIR detecta movimento
  - Câmera ativada sob demanda para economia de energia
  - Múltiplos modos de operação (Hibernação/Auto/Manual/Vigilância)
  - Frames enviados via MQTT para análise de IA no servidor CCO
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
#include "PIRSensor.h"
#include "PowerManager.h"
#include "OperationMode.h"
#include "MQTTPublisher.h"
#include "http_server.h"

// =================== Instâncias globais ===================
WiFiConnector wifiConnector;
CameraController cameraController;
PIRSensor pirSensor;
PowerManager powerManager;
OperationModeController modeController;
MQTTPublisher mqttPublisher;

// =================== Variáveis de estado ===================
unsigned long lastMotionDetection = 0;
unsigned long lastVigilanceCapture = 0;
bool cameraInitialized = false;

// =================== Callback para mudança de modo via MQTT ===================
void onModeChangedCallback(OperationMode newMode)
{
  modeController.setMode(newMode);
  String modeStr = modeController.getModeString();
  modeStr.toLowerCase();
  mqttPublisher.publishMode(modeStr);
  
  // Ajustar estado da câmera baseado no novo modo
  if (newMode == OperationMode::HIBERNATION || newMode == OperationMode::MANUAL)
  {
    if (cameraInitialized)
    {
      cameraController.powerOff();
      cameraInitialized = false;
    }
    powerManager.enterHibernation();
  }
  else if (newMode == OperationMode::VIGILANCE)
  {
    // Modo vigília mantém câmera ativa
    if (!cameraInitialized)
    {
      cameraController.powerOn();
      if (cameraController.begin())
      {
        cameraInitialized = true;
      }
    }
  }
}

// =================== Callback para status do PIR via MQTT ===================
void onPIRStatusRequestedCallback()
{
  pirSensor.printStatus();
}

// =================== Setup ===================
void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println("========================================");
  Serial.println("  Sistema de Monitoramento Ferroviário");
  Serial.println("========================================");

  // Inicializar sensor PIR
  pirSensor.begin();

  // Conectar WiFi
  if (!wifiConnector.connect(WIFI_SSID, WIFI_PASS))
  {
    Serial.println("[Setup] Wi-Fi indisponível. Reiniciando...");
    ESP.restart();
  }

  // Inicializar MQTT
  if (mqttPublisher.begin())
  {
    Serial.println("[MQTT] Inicializado. Tentando conectar...");
  }
  else
  {
    Serial.println("[MQTT] Desabilitado ou falha na inicialização");
  }

  // Iniciar em modo HIBERNATION por padrão (economia de energia)
  modeController.setMode(OperationMode::HIBERNATION);
  powerManager.enterHibernation();
  mqttPublisher.publishMode("hibernation");

  // Inicializar servidor HTTP (para monitoramento)
  startCameraServer();

  Serial.println("\n========================================");
  Serial.println("  Sistema Pronto!");
  Serial.println("========================================");
  Serial.println("Servidor HTTP: http://" + WiFi.localIP().toString());
  Serial.println("MQTT Broker: " + String(MQTT_BROKER));
  Serial.println("Modo inicial: HIBERNATION");
  Serial.println("========================================\n");
}

// =================== Loop principal ===================
void loop()
{
  // Manter conexão MQTT ativa e processar mensagens
  mqttPublisher.loop();

  // Verificar se há solicitação de captura manual
  if (modeController.isManual() && mqttPublisher.isManualCaptureRequested())
  {
    handleManualCapture();
  }
  
  // Verificar comandos MQTT que podem mudar o modo
  // (processado internamente no mqttPublisher.loop())

  // Lógica baseada no modo atual
  switch (modeController.getMode())
  {
    case OperationMode::HIBERNATION:
      handleHibernationMode();
      break;

    case OperationMode::AUTO:
      handleAutoMode();
      break;

    case OperationMode::MANUAL:
      handleManualMode();
      break;

    case OperationMode::VIGILANCE:
      handleVigilanceMode();
      break;
  }

  delay(10); // Pequeno delay para não sobrecarregar
}

// =================== Handlers de Modos ===================

void handleHibernationMode()
{
  // Apenas monitora PIR, câmera desligada
  if (pirSensor.isMotionDetected())
  {
    // Notificar CCO sobre movimento (mas não ligar câmera)
    mqttPublisher.publishMotionDetected();
    Serial.println("[Hibernation] Movimento detectado - notificação enviada ao CCO");
  }
}

void handleAutoMode()
{
  // Monitora PIR, quando detecta movimento → liga câmera → captura → envia → desliga
  if (pirSensor.isMotionDetected())
  {
    unsigned long now = millis();
    
    // Cooldown para evitar múltiplas capturas muito próximas
    if (now - lastMotionDetection < AUTO_MODE_COOLDOWN_MS)
    {
      return;
    }

    lastMotionDetection = now;
    Serial.println("[Auto] Movimento detectado - capturando frame...");

    // Notificar CCO sobre movimento
    mqttPublisher.publishMotionDetected();

    // Ligar câmera se necessário
    if (!cameraInitialized)
    {
      cameraController.powerOn();
      if (!cameraController.begin())
      {
        Serial.println("[Auto] Falha ao inicializar câmera");
        return;
      }
      cameraInitialized = true;
      delay(CAMERA_STABILIZATION_MS);
    }

    // Capturar frame
    camera_fb_t *fb = cameraController.captureFrame();
    if (fb != nullptr)
    {
      // Enviar frame via MQTT com flag motion_triggered
      mqttPublisher.publishFrame(fb, "auto", true);
      cameraController.returnFrame(fb);
    }

    // Desligar câmera para economia de energia
    // Nota: Mantemos inicializada para evitar delay na próxima captura
    // Mas podemos desligar fisicamente se necessário
  }
}

void handleManualMode()
{
  // Câmera desligada, aguarda comando MQTT
  // A captura é tratada no loop principal quando detecta requestManualCapture
  if (cameraInitialized)
  {
    cameraController.powerOff();
    cameraInitialized = false;
  }
}

void handleManualCapture()
{
  Serial.println("[Manual] Captura manual solicitada...");

  // Ligar câmera se necessário
  if (!cameraInitialized)
  {
    cameraController.powerOn();
    if (!cameraController.begin())
    {
      Serial.println("[Manual] Falha ao inicializar câmera");
      return;
    }
    cameraInitialized = true;
    delay(CAMERA_STABILIZATION_MS);
  }

  // Capturar frame
  camera_fb_t *fb = cameraController.captureFrame();
  if (fb != nullptr)
  {
    // Enviar frame via MQTT
    mqttPublisher.publishFrame(fb, "manual", false);
    cameraController.returnFrame(fb);
  }

  // Desligar câmera após captura
  cameraController.powerOff();
  cameraInitialized = false;
}

void handleVigilanceMode()
{
  // Câmera ativa, captura frames em intervalos
  unsigned long now = millis();

  if (now - lastVigilanceCapture >= VIGILANCE_INTERVAL_MS)
  {
    lastVigilanceCapture = now;

    // Garantir que câmera está inicializada
    if (!cameraInitialized)
    {
      cameraController.powerOn();
      if (!cameraController.begin())
      {
        Serial.println("[Vigilance] Falha ao inicializar câmera");
        return;
      }
      cameraInitialized = true;
      delay(CAMERA_STABILIZATION_MS);
    }

    // Capturar frame
    camera_fb_t *fb = cameraController.captureFrame();
    if (fb != nullptr)
    {
      // Enviar frame via MQTT
      mqttPublisher.publishFrame(fb, "vigilance", false);
      cameraController.returnFrame(fb);
    }
  }
}
