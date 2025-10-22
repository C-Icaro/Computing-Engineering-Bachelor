/*
 * EasyWay Client - Sistema Completo (Pinos Seguros) - CORRIGIDO
 * Sistema de comunicação IoT via Wi-Fi
 */

#include <WiFi.h>
#include <WiFiUDP.h>
#include <ArduinoJson.h>

// Configurações do dispositivo
const String DEVICE_ID = "cliente01";
const char* WIFI_SSID = "EasyWay-Server";
const char* WIFI_PASSWORD = "easyway123";
const int SERVER_PORT = 8080;

// Pinos SEGUROS para ESP32 (evitando GPIO6-11 que são SPI Flash)
const int LED_CONNECTED_PIN = 2;    // Verde - Conectado (GPIO2 é seguro)
const int LED_ERROR_PIN = 4;        // Vermelho - Erro (GPIO4 é seguro)
const int LED_TRANSMIT_PIN = 5;     // Amarelo - Transmitindo (GPIO5 é seguro)
const int DOOR_RELAY_PIN = 13;      // Relé para controle da porta (GPIO13 é seguro)
const int BUTTON_PIN = 14;          // Botão para comandos locais (GPIO14 é seguro)

// Instâncias dos módulos
WiFiUDP udp;

// Variáveis de estado
bool isConnected = false;
bool doorOpen = false;
unsigned long lastHeartbeat = 0;
unsigned long lastTransmission = 0;
const unsigned long HEARTBEAT_INTERVAL = 30000; // 30 segundos
const unsigned long TRANSMISSION_LED_TIME = 200; // 200ms para LED de transmissão

void setup() {
  Serial.begin(115200);
  Serial.println("EasyWay Client - Sistema Completo (Pinos Seguros) - CORRIGIDO");
  
  // Inicializar pinos SEGUROS
  pinMode(DOOR_RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_CONNECTED_PIN, OUTPUT);
  pinMode(LED_ERROR_PIN, OUTPUT);
  pinMode(LED_TRANSMIT_PIN, OUTPUT);
  
  // Estado inicial seguro - TODOS OS LEDs APAGADOS
  digitalWrite(DOOR_RELAY_PIN, LOW);
  digitalWrite(LED_CONNECTED_PIN, LOW);
  digitalWrite(LED_ERROR_PIN, LOW);
  digitalWrite(LED_TRANSMIT_PIN, LOW);
  
  Serial.println("Pinos configurados:");
  Serial.println("- LED Conectado: GPIO" + String(LED_CONNECTED_PIN));
  Serial.println("- LED Erro: GPIO" + String(LED_ERROR_PIN));
  Serial.println("- LED Transmitindo: GPIO" + String(LED_TRANSMIT_PIN));
  Serial.println("- Relé Porta: GPIO" + String(DOOR_RELAY_PIN));
  Serial.println("- Botão Local: GPIO" + String(BUTTON_PIN));
  
  // Conectar ao Wi-Fi
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Wi-Fi conectado!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    if (udp.begin(SERVER_PORT)) {
      Serial.print("UDP Client iniciado na porta: ");
      Serial.println(SERVER_PORT);
      isConnected = true;
      // LED verde liga apenas quando conectado
      digitalWrite(LED_CONNECTED_PIN, HIGH);
      Serial.println("LED Verde: LIGADO (conectado)");
    } else {
      Serial.println("Falha ao iniciar UDP Client!");
      digitalWrite(LED_ERROR_PIN, HIGH);
      Serial.println("LED Vermelho: LIGADO (erro UDP)");
    }
  } else {
    Serial.println("Falha ao conectar ao Wi-Fi!");
    digitalWrite(LED_ERROR_PIN, HIGH);
    Serial.println("LED Vermelho: LIGADO (erro Wi-Fi)");
  }
  
  Serial.println("EasyWay Client - Dispositivo inicializado!");
  Serial.print("ID do dispositivo: ");
  Serial.println(DEVICE_ID);
  
  // Enviar sinal de presença
  sendPresenceSignal();
}

void loop() {
  // Verificar botão local
  checkLocalButton();
  
  // Processar mensagens recebidas
  processWiFiMessages();
  
  // Enviar heartbeat periódico
  sendHeartbeat();
  
  // Gerenciar LEDs
  updateLEDs();
  
  delay(100);
}

void sendPresenceSignal() {
  String message = "{\"id\":\"" + DEVICE_ID + "\",\"type\":\"status\",\"status\":\"online\"}";
  sendMessage(message);
  Serial.println("Sinal de presença enviado");
}

void processWiFiMessages() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.print("Client received from ");
    Serial.print(udp.remoteIP());
    Serial.print(":");
    Serial.print(udp.remotePort());
    Serial.print(" - ");
    Serial.println(incomingPacket);
    
    // Processar comando simples
    if (strstr(incomingPacket, "abrir") != NULL) {
      openDoor();
      Serial.println("Comando recebido: ABRIR porta");
    } else if (strstr(incomingPacket, "fechar") != NULL) {
      closeDoor();
      Serial.println("Comando recebido: FECHAR porta");
    }
  }
}

void openDoor() {
  digitalWrite(DOOR_RELAY_PIN, HIGH);
  doorOpen = true;
  Serial.println("Porta ABERTA (relé ativado)");
  delay(1000); // Manter aberto por 1 segundo
  digitalWrite(DOOR_RELAY_PIN, LOW);
  doorOpen = false;
  Serial.println("Porta FECHADA (relé desativado)");
}

void closeDoor() {
  digitalWrite(DOOR_RELAY_PIN, LOW);
  doorOpen = false;
  Serial.println("Porta FECHADA");
}

void sendHeartbeat() {
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    String message = "{\"id\":\"" + DEVICE_ID + "\",\"type\":\"heartbeat\",\"status\":\"ping\"}";
    sendMessage(message);
    lastHeartbeat = millis();
    Serial.println("Heartbeat enviado");
  }
}

void sendMessage(String message) {
  IPAddress serverIP(192, 168, 4, 1); // IP padrão do ESP32 softAP
  
  udp.beginPacket(serverIP, SERVER_PORT);
  udp.print(message);
  udp.endPacket();
  Serial.print("Client sent: ");
  Serial.println(message);
  
  // Ativar LED de transmissão
  digitalWrite(LED_TRANSMIT_PIN, HIGH);
  lastTransmission = millis();
  Serial.println("LED Amarelo: LIGADO (transmitindo)");
}

void checkLocalButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce
    
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("Botão pressionado!");
      
      // Alternar estado da porta
      if (doorOpen) {
        closeDoor();
        Serial.println("Botão local: FECHAR porta");
      } else {
        openDoor();
        Serial.println("Botão local: ABRIR porta");
      }
      
      // Enviar status atualizado
      String status = doorOpen ? "aberto" : "fechado";
      String message = "{\"id\":\"" + DEVICE_ID + "\",\"type\":\"status\",\"status\":\"" + status + "\"}";
      sendMessage(message);
      
      // Aguardar soltar o botão
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
      Serial.println("Botão solto");
    }
  }
}

void updateLEDs() {
  // Desligar LED de transmissão após tempo determinado
  if (digitalRead(LED_TRANSMIT_PIN) == HIGH && millis() - lastTransmission > TRANSMISSION_LED_TIME) {
    digitalWrite(LED_TRANSMIT_PIN, LOW);
    Serial.println("LED Amarelo: APAGADO (transmissão concluída)");
  }
  
  // Verificar conexão e atualizar LED verde
  if (isConnected && WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_CONNECTED_PIN, HIGH);
    digitalWrite(LED_ERROR_PIN, LOW);
  } else {
    digitalWrite(LED_CONNECTED_PIN, LOW);
    digitalWrite(LED_ERROR_PIN, HIGH);
    isConnected = false;
  }
}