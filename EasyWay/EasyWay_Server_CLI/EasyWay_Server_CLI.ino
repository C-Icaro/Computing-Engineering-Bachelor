/*
 * EasyWay Server - Dispositivo Servidor
 * Sistema de comunicação IoT via Wi-Fi com plataforma web local
 * 
 * Funcionalidades:
 * - Comunicação via Wi-Fi direta
 * - Servidor web embutido
 * - Controle remoto de portas
 * - Logs de comunicação
 * - Interface web responsiva
 */

#include "WiFiCommunication.h"
#include "WebServer.h"
#include "SecurityLayer.h"
#include "DeviceManager.h"

// Configurações do sistema
const char* WIFI_SSID = "EasyWay-Server";
const char* WIFI_PASSWORD = "easyway123";
const int LED_STATUS_PIN = 2;
const int SERVER_PORT = 8080;

// Instâncias dos módulos
WiFiCommunication wifiComm;
WebServerManager webServer;
SecurityLayer security;
DeviceManager deviceManager;

void setup() {
  Serial.begin(115200);
  Serial.println("EasyWay Server - Iniciando sistema...");
  
  // Inicializar LED de status
  pinMode(LED_STATUS_PIN, OUTPUT);
  digitalWrite(LED_STATUS_PIN, LOW);
  
  // Inicializar comunicação via Wi-Fi
  if (!wifiComm.initialize(WIFI_SSID, WIFI_PASSWORD, SERVER_PORT)) {
    Serial.println("ERRO: Falha ao inicializar comunicação via Wi-Fi");
    return;
  }
  
  // Inicializar camada de segurança
  security.initialize();
  
  // Inicializar gerenciador de dispositivos
  deviceManager.initialize();
  
  // Inicializar servidor web
  if (!webServer.initialize(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("ERRO: Falha ao inicializar servidor web");
    return;
  }
  
  // Configurar rotas da API
  setupWebRoutes();
  
  Serial.println("EasyWay Server - Sistema inicializado com sucesso!");
  Serial.print("Acesse: http://");
  Serial.println(WiFi.softAPIP());
  Serial.print("Porta de comunicação: ");
  Serial.println(SERVER_PORT);
  
  digitalWrite(LED_STATUS_PIN, HIGH);
}

void loop() {
  // Processar mensagens recebidas via Wi-Fi
  processWiFiMessages();
  
  // Processar comandos da interface web
  webServer.handleClient();
  
  // Atualizar status dos dispositivos
  deviceManager.updateDeviceStatus();
  
  delay(100);
}

void setupWebRoutes() {
  // Rotas da API REST
  webServer.addRoute("/api/devices", "GET", handleGetDevices);
  webServer.addRoute("/api/devices/{id}/control", "POST", handleControlDevice);
  webServer.addRoute("/api/message", "POST", handleSendMessage);
  webServer.addRoute("/api/logs", "GET", handleGetLogs);
  
  // Servir arquivos estáticos
  webServer.serveStatic("/", "/index.html");
  webServer.serveStatic("/style.css", "/style.css");
  webServer.serveStatic("/script.js", "/script.js");
}

void processWiFiMessages() {
  if (wifiComm.hasMessage()) {
    String message = wifiComm.receiveMessage();
    
    if (security.validateMessage(message)) {
      JsonObject data = security.parseMessage(message);
      
      if (data["type"] == "command") {
        handleDeviceCommand(data);
      } else if (data["type"] == "status") {
        handleDeviceStatus(data);
      } else if (data["type"] == "message") {
        handleClientMessage(data);
      }
      
      // Registrar log
      deviceManager.addLog(data["id"], data["cmd"], millis());
      
      // Enviar confirmação
      String ack = security.createAckMessage(data["id"]);
      wifiComm.sendMessage(data["id"], ack);
    }
  }
}

void handleDeviceCommand(JsonObject data) {
  String deviceId = data["id"];
  String command = data["cmd"];
  
  if (command == "abrir" || command == "fechar") {
    deviceManager.executeCommand(deviceId, command);
  }
}

void handleDeviceStatus(JsonObject data) {
  String deviceId = data["id"];
  String status = data["status"];
  
  deviceManager.updateDeviceStatus(deviceId, status);
}

void handleClientMessage(JsonObject data) {
  String deviceId = data["id"];
  String message = data["message"];
  
  deviceManager.addMessage(deviceId, message);
}

// Handlers da API REST
void handleGetDevices() {
  String devicesJson = deviceManager.getDevicesJson();
  webServer.sendResponse(200, "application/json", devicesJson);
}

void handleControlDevice() {
  String deviceId = webServer.getPathParameter("id");
  String command = webServer.getBodyParameter("command");
  
  if (deviceManager.executeCommand(deviceId, command)) {
    webServer.sendResponse(200, "application/json", "{\"status\":\"success\"}");
  } else {
    webServer.sendResponse(400, "application/json", "{\"status\":\"error\"}");
  }
}

void handleSendMessage() {
  String deviceId = webServer.getBodyParameter("deviceId");
  String message = webServer.getBodyParameter("message");
  
  String wifiMessage = security.createMessage(deviceId, "message", message);
  wifiComm.sendMessage(deviceId, wifiMessage);
  
  webServer.sendResponse(200, "application/json", "{\"status\":\"sent\"}");
}

void handleGetLogs() {
  String logsJson = deviceManager.getLogsJson();
  webServer.sendResponse(200, "application/json", logsJson);
}
