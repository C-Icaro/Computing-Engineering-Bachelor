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
#include "WebServerManager.h"
#include "SecurityLayer.h"
#include "DeviceManager.h"

// Configurações do sistema
const char* WIFI_SSID = "EasyWay-Server";           // Nome da sua rede
const char* WIFI_PASSWORD = "easyway123";           // Senha da sua rede
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
  
  // Configurar referências para comunicação
  webServer.setReferences(&wifiComm, &security, &deviceManager);
  
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
  
  // Processar comandos via Serial (para teste)
  processSerialCommands();
  
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
    Serial.println("=== MENSAGEM UDP RECEBIDA NO SERVIDOR ===");
    String message = wifiComm.receiveMessage();
    Serial.print("Mensagem recebida: ");
    Serial.println(message);
    
    if (security.validateMessage(message)) {
      Serial.println("Mensagem validada com sucesso");
      JsonObject data = security.parseMessage(message);
      String deviceId = data["id"];
      String messageType = data["type"];
      
      Serial.print("Device ID: ");
      Serial.println(deviceId);
      Serial.print("Tipo da mensagem: ");
      Serial.println(messageType);
      
      // Capturar IP do cliente
      IPAddress clientIP = wifiComm.getRemoteIP();
      Serial.print("IP do cliente capturado: ");
      Serial.println(clientIP);
      
      if (clientIP != IPAddress(0, 0, 0, 0)) {
        deviceManager.updateDeviceIP(deviceId, clientIP);
        Serial.print("IP do cliente registrado: ");
        Serial.print(deviceId);
        Serial.print(" -> ");
        Serial.println(clientIP);
      }
      
      if (messageType == "command") {
        Serial.println("Processando comando do dispositivo");
        handleDeviceCommand(data);
      } else if (messageType == "status") {
        Serial.println("Processando status do dispositivo");
        handleDeviceStatus(data);
      } else if (messageType == "message") {
        Serial.println("Processando mensagem do dispositivo");
        handleClientMessage(data);
      }
      
      // Registrar log
      String logCommand = data["cmd"].as<String>();
      if (logCommand.length() == 0) {
        logCommand = data["type"].as<String>();
      }
      deviceManager.addLog(deviceId, logCommand, millis());
      
      // Enviar confirmação para o IP correto
      String ack = security.createAckMessage(deviceId);
      Serial.print("Enviando confirmação para: ");
      Serial.println(clientIP);
      
      if (clientIP != IPAddress(0, 0, 0, 0)) {
        wifiComm.sendMessageToIP(clientIP, deviceId, ack);
      } else {
        wifiComm.sendMessage(deviceId, ack);
      }
    } else {
      Serial.println("ERRO: Mensagem não validada");
    }
    Serial.println("=== FIM DO PROCESSAMENTO ===");
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
    // Obter IP do cliente
    IPAddress clientIP = deviceManager.getDeviceIP(deviceId);
    
    // Enviar comando via WiFi para o dispositivo
    String wifiMessage = security.createMessage(deviceId, "command", command);
    bool sent = false;
    
    if (clientIP != IPAddress(0, 0, 0, 0)) {
      sent = wifiComm.sendMessageToIP(clientIP, deviceId, wifiMessage);
      Serial.print("Comando enviado via WiFi para IP específico ");
      Serial.print(clientIP);
      Serial.print(" (");
      Serial.print(deviceId);
      Serial.print("): ");
      Serial.println(command);
    } else {
      sent = wifiComm.sendMessage(deviceId, wifiMessage);
      Serial.print("AVISO: IP não encontrado, enviando para IP padrão (");
      Serial.print(deviceId);
      Serial.print("): ");
      Serial.println(command);
    }
    
    if (sent) {
      webServer.sendResponse(200, "application/json", "{\"status\":\"success\",\"sent\":true}");
    } else {
      Serial.print("ERRO: Falha ao enviar comando via WiFi para ");
      Serial.println(deviceId);
      webServer.sendResponse(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to send command\"}");
    }
  } else {
    webServer.sendResponse(400, "application/json", "{\"status\":\"error\",\"message\":\"Device not found\"}");
  }
}

void handleSendMessage() {
  String deviceId = webServer.getBodyParameter("deviceId");
  String message = webServer.getBodyParameter("message");
  
  // Obter IP do cliente
  IPAddress clientIP = deviceManager.getDeviceIP(deviceId);
  
  String wifiMessage = security.createMessage(deviceId, "message", message);
  bool sent = false;
  
  if (clientIP != IPAddress(0, 0, 0, 0)) {
    sent = wifiComm.sendMessageToIP(clientIP, deviceId, wifiMessage);
  } else {
    sent = wifiComm.sendMessage(deviceId, wifiMessage);
  }
  
  webServer.sendResponse(200, "application/json", "{\"status\":\"sent\"}");
}

void handleGetLogs() {
  String logsJson = deviceManager.getLogsJson();
  webServer.sendResponse(200, "application/json", logsJson);
}

void processSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "list") {
      deviceManager.printDeviceList();
    } else if (command == "logs") {
      deviceManager.printLogs();
    } else if (command.startsWith("send ")) {
      // Formato: send cliente01 abrir
      int firstSpace = command.indexOf(' ');
      int secondSpace = command.indexOf(' ', firstSpace + 1);
      
      if (firstSpace > 0 && secondSpace > firstSpace) {
        String deviceId = command.substring(firstSpace + 1, secondSpace);
        String cmd = command.substring(secondSpace + 1);
        
        Serial.print("Enviando comando via Serial: ");
        Serial.print(deviceId);
        Serial.print(" -> ");
        Serial.println(cmd);
        
        // Obter IP do cliente
        IPAddress clientIP = deviceManager.getDeviceIP(deviceId);
        
        // Enviar comando via WiFi
        String wifiMessage = security.createMessage(deviceId, "command", cmd);
        bool sent = false;
        
        if (clientIP != IPAddress(0, 0, 0, 0)) {
          sent = wifiComm.sendMessageToIP(clientIP, deviceId, wifiMessage);
          Serial.print("Comando enviado para IP específico: ");
          Serial.println(clientIP);
        } else {
          sent = wifiComm.sendMessage(deviceId, wifiMessage);
          Serial.println("AVISO: IP não encontrado, usando IP padrão");
        }
        
        if (sent) {
          Serial.println("Comando enviado com sucesso!");
          deviceManager.executeCommand(deviceId, cmd);
        } else {
          Serial.println("ERRO: Falha ao enviar comando!");
        }
      } else {
        Serial.println("Formato: send <deviceId> <command>");
      }
    } else if (command == "devices") {
      Serial.println("=== DISPOSITIVOS CONECTADOS ===");
      deviceManager.printDeviceList();
    } else if (command == "help") {
      Serial.println("Comandos disponíveis:");
      Serial.println("  list - Lista dispositivos");
      Serial.println("  devices - Lista dispositivos (formato detalhado)");
      Serial.println("  logs - Mostra logs");
      Serial.println("  send <deviceId> <command> - Envia comando");
      Serial.println("  help - Mostra esta ajuda");
    } else {
      Serial.println("Comando não reconhecido. Digite 'help' para ajuda.");
    }
  }
}
