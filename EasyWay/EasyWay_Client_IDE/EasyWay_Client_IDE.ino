/*
 * EasyWay Client - Sistema Completo (Pinos Seguros) - CORRIGIDO
 * Sistema de comunicação IoT via Wi-Fi
 */

#include <WiFi.h>
#include <WiFiUDP.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

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
Adafruit_BME280 bme; // Instância do sensor BME280

// Configurações do BME280
#define BME280_ADDRESS 0x76
const float SEALEVEL_HPA = 1013.25; // Pressão ao nível do mar para cálculo de altitude

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
  
  // Inicializar I2C e BME280
  Wire.begin();
  Serial.println("Inicializando sensor BME280...");
  
  if (!bme.begin(BME280_ADDRESS)) {
    Serial.println("ERRO: Sensor BME280 não encontrado!");
    Serial.println("Verifique a fiação (VCC, GND, SDA, SCL) e alimentação.");
    digitalWrite(LED_ERROR_PIN, HIGH);
    Serial.println("LED Vermelho: LIGADO (erro BME280)");
  } else {
    Serial.println("BME280 inicializado com sucesso!");
    Serial.print("ID do sensor: 0x");
    Serial.println(bme.sensorID(), HEX);
    
    // Configurar sensor para modo normal com configurações recomendadas
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X2,  // Temperatura oversampling
                    Adafruit_BME280::SAMPLING_X16, // Pressão oversampling
                    Adafruit_BME280::SAMPLING_X1,  // Umidade oversampling
                    Adafruit_BME280::FILTER_X16,   // Filtro IIR
                    Adafruit_BME280::STANDBY_MS_500);
    
    Serial.println("Configuração do BME280 concluída!");
  }
  
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
    
    // Processar comandos simples
    if (strstr(incomingPacket, "abrir") != NULL) {
      openDoor();
      Serial.println("Comando recebido: ABRIR porta");
    } else if (strstr(incomingPacket, "fechar") != NULL) {
      closeDoor();
      Serial.println("Comando recebido: FECHAR porta");
    } else if (strstr(incomingPacket, "temperatura") != NULL || strstr(incomingPacket, "temp") != NULL) {
      String sensorData = readBME280Data();
      sendMessage(sensorData);
      Serial.println("Comando recebido: DADOS DO SENSOR");
    } else if (strstr(incomingPacket, "status") != NULL) {
      String status = doorOpen ? "aberto" : "fechado";
      String message = "{\"id\":\"" + DEVICE_ID + "\",\"type\":\"status\",\"status\":\"" + status + "\"}";
      sendMessage(message);
      Serial.println("Comando recebido: STATUS DA PORTA");
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
    // Enviar heartbeat básico
    String heartbeatMessage = "{\"id\":\"" + DEVICE_ID + "\",\"type\":\"heartbeat\",\"status\":\"ping\"}";
    sendMessage(heartbeatMessage);
    
    // Enviar dados do sensor BME280 junto com o heartbeat
    String sensorData = readBME280Data();
    sendMessage(sensorData);
    
    lastHeartbeat = millis();
    Serial.println("Heartbeat e dados do sensor enviados");
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

String readBME280Data() {
  // Ler todos os valores do sensor
  float temperature = bme.readTemperature();        // °C
  float humidity = bme.readHumidity();              // %
  float pressure = bme.readPressure() / 100.0F;     // hPa
  float altitude = bme.readAltitude(SEALEVEL_HPA);  // metros

  // Criar JSON com os dados do sensor
  DynamicJsonDocument doc(512);
  doc["id"] = DEVICE_ID;
  doc["type"] = "sensor_data";
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["pressure"] = pressure;
  doc["altitude"] = altitude;
  doc["timestamp"] = millis();

  String jsonString;
  serializeJson(doc, jsonString);
  
  // Log dos dados lidos
  Serial.println("--- Dados BME280 ---");
  Serial.print("Temperatura: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");
  Serial.print("Umidade: ");
  Serial.print(humidity, 2);
  Serial.println(" %");
  Serial.print("Pressão: ");
  Serial.print(pressure, 2);
  Serial.println(" hPa");
  Serial.print("Altitude aproximada: ");
  Serial.print(altitude, 2);
  Serial.println(" m");
  Serial.println("-------------------");
  
  return jsonString;
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