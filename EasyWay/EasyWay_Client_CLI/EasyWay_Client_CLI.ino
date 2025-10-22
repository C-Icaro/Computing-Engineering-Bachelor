/*
 * EasyWay Client CLI - Dispositivo Cliente
 * Sistema de comunicação IoT via Wi-Fi com arquitetura modular
 * 
 * Funcionalidades:
 * - Comunicação via Wi-Fi UDP
 * - Controle de dispositivos físicos
 * - Interface de LEDs e botões
 * - Heartbeat automático
 * - Comandos locais e remotos
 */

#include "WiFiCommunication.h"
#include "DeviceController.h"
#include "SecurityLayer.h"
#include "StatusManager.h"

// Configurações do sistema
const char* WIFI_SSID = "EasyWay-Server";           // Nome da rede do servidor
const char* WIFI_PASSWORD = "easyway123";           // Senha da rede do servidor
const int SERVER_PORT = 8080;
const String DEVICE_ID = "cliente01";

// Instâncias dos módulos
WiFiCommunication wifiComm;
DeviceController deviceController;
SecurityLayer security;
StatusManager statusManager;

void setup() {
  Serial.begin(115200);
  Serial.println("EasyWay Client CLI - Iniciando sistema...");
  
  // Inicializar camada de segurança
  security.initialize();
  
  // Inicializar controlador de dispositivos
  deviceController.initialize();
  Serial.println("DeviceController inicializado");
  
  // Inicializar gerenciador de status com referência ao DeviceController
  statusManager.initialize(&deviceController);
  Serial.println("StatusManager inicializado com DeviceController");
  
  // Inicializar comunicação via Wi-Fi
  if (!wifiComm.initialize(WIFI_SSID, WIFI_PASSWORD, SERVER_PORT)) {
    Serial.println("ERRO: Falha ao inicializar comunicação via Wi-Fi");
    statusManager.setError("WiFi initialization failed");
    return;
  }
  
  Serial.println("EasyWay Client CLI - Sistema inicializado com sucesso!");
  Serial.print("ID do dispositivo: ");
  Serial.println(DEVICE_ID);
  Serial.print("Conectado à rede: ");
  Serial.println(WIFI_SSID);
  
  // Enviar sinal de presença
  sendPresenceSignal();
  
  statusManager.setConnected(true);
  
  // Teste inicial dos LEDs
  Serial.println("=== TESTE INICIAL DOS LEDs ===");
  deviceController.setConnectedLED(true);
  delay(1000);
  deviceController.setConnectedLED(false);
  deviceController.setErrorLED(true);
  delay(1000);
  deviceController.setErrorLED(false);
  Serial.println("=== FIM DO TESTE INICIAL ===");
}

void loop() {
  // Processar mensagens recebidas via Wi-Fi
  processWiFiMessages();
  
  // Verificar comandos locais (botões)
  deviceController.checkLocalCommands();
  
  // Enviar heartbeat periódico
  sendHeartbeat();
  
  // Atualizar status dos LEDs
  statusManager.updateStatus();
  
  // Processar comandos via Serial (para teste)
  processSerialCommands();
  
  delay(100);
}

void sendPresenceSignal() {
  String message = security.createMessage(DEVICE_ID, "status", "online");
  wifiComm.sendMessage(DEVICE_ID, message);
  Serial.println("Sinal de presença enviado");
}

void processWiFiMessages() {
  if (wifiComm.hasMessage()) {
    String message = wifiComm.receiveMessage();
    Serial.println("=== MENSAGEM RECEBIDA ===");
    Serial.println("Raw message: " + message);
    
    if (security.validateMessage(message)) {
      Serial.println("Mensagem validada com sucesso");
      JsonObject data = security.parseMessage(message);
      
      Serial.print("Tipo da mensagem: ");
      Serial.println(data["type"].as<String>());
      Serial.print("ID: ");
      Serial.println(data["id"].as<String>());
      
      if (data["type"] == "command") {
        Serial.print("Comando recebido: ");
        Serial.println(data["cmd"].as<String>());
        handleDeviceCommand(data);
      } else if (data["type"] == "status_request") {
        Serial.println("Solicitação de status recebida");
        handleStatusRequest(data);
      }
      
      // Enviar confirmação
      String ack = security.createAckMessage(data["id"]);
      wifiComm.sendMessage(data["id"], ack);
      Serial.println("Confirmação enviada");
    } else {
      Serial.println("ERRO: Mensagem não validada");
    }
    Serial.println("=== FIM DA MENSAGEM ===");
  }
}

void handleDeviceCommand(JsonObject data) {
  String command = data["cmd"];
  
  Serial.println("=== EXECUTANDO COMANDO ===");
  Serial.print("Comando recebido: ");
  Serial.println(command);
  
  if (command == "abrir") {
    Serial.println("Executando comando: ABRIR PORTA");
    deviceController.openDoor();
    sendStatusUpdate("aberto");
    Serial.println("Comando ABRIR executado com sucesso");
  } else if (command == "fechar") {
    Serial.println("Executando comando: FECHAR PORTA");
    deviceController.closeDoor();
    sendStatusUpdate("fechado");
    Serial.println("Comando FECHAR executado com sucesso");
  } else if (command == "status") {
    Serial.println("Executando comando: STATUS");
    sendStatusUpdate(deviceController.getDoorStatus());
    Serial.println("Comando STATUS executado com sucesso");
  } else {
    Serial.print("Comando não reconhecido: ");
    Serial.println(command);
  }
  
  Serial.println("=== FIM DO COMANDO ===");
}

void handleStatusRequest(JsonObject data) {
  String status = deviceController.getDoorStatus();
  sendStatusUpdate(status);
  Serial.println("Status enviado: " + status);
}

void sendStatusUpdate(String status) {
  String message = security.createMessage(DEVICE_ID, "status", status);
  wifiComm.sendMessage(DEVICE_ID, message);
}

void sendHeartbeat() {
  static unsigned long lastHeartbeat = 0;
  const unsigned long HEARTBEAT_INTERVAL = 30000; // 30 segundos
  
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    String message = security.createMessage(DEVICE_ID, "heartbeat", "ping");
    wifiComm.sendMessage(DEVICE_ID, message);
    lastHeartbeat = millis();
    Serial.println("Heartbeat enviado");
  }
}

void processSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "test") {
      Serial.println("=== TESTE DO SISTEMA ===");
      deviceController.testRelay();
      Serial.println("Teste de LEDs:");
      deviceController.setConnectedLED(true);
      delay(1000);
      deviceController.setConnectedLED(false);
      deviceController.setErrorLED(true);
      delay(1000);
      deviceController.setErrorLED(false);
      deviceController.setTransmitLED(true);
      delay(1000);
      deviceController.setTransmitLED(false);
      Serial.println("=== FIM DO TESTE ===");
    } else if (command == "status") {
      statusManager.printStatus();
    } else if (command == "open") {
      deviceController.openDoor();
    } else if (command == "close") {
      deviceController.closeDoor();
    } else if (command == "help") {
      Serial.println("Comandos disponíveis:");
      Serial.println("  test - Testa relé e LEDs");
      Serial.println("  status - Mostra status do sistema");
      Serial.println("  open - Abre a porta");
      Serial.println("  close - Fecha a porta");
      Serial.println("  help - Mostra esta ajuda");
    } else {
      Serial.println("Comando não reconhecido. Digite 'help' para ajuda.");
    }
  }
}
