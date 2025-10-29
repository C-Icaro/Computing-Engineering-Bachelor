/*
 * EasyWay Server - Sistema Completo com Web Server
 * Sistema de comunicação IoT via Wi-Fi com interface web
 */

#include <WiFi.h>
#include <WiFiUDP.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Configurações do sistema
const char* WIFI_SSID = "EasyWay-Server";
const char* WIFI_PASSWORD = "easyway123";
const int LED_STATUS_PIN = 2;
const int UDP_PORT = 8080;
const int WEB_PORT = 80;

// Instâncias dos módulos
WiFiUDP udp;
WebServer webServer(WEB_PORT);

// Variáveis de estado
String clientStatus = "Desconectado";
String lastMessage = "Nenhuma mensagem";
unsigned long lastHeartbeat = 0;

// Variáveis para dados do BME280
float lastTemperature = 0.0;
float lastHumidity = 0.0;
float lastPressure = 0.0;
float lastAltitude = 0.0;
unsigned long lastSensorUpdate = 0;
String sensorStatus = "Sem dados";

void setup() {
  Serial.begin(115200);
  Serial.println("EasyWay Server - Sistema Completo");
  
  // Inicializar LED de status
  pinMode(LED_STATUS_PIN, OUTPUT);
  digitalWrite(LED_STATUS_PIN, LOW);
  
  // Inicializar Wi-Fi como Access Point
  Serial.print("Configurando Wi-Fi como Access Point: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Inicializar servidor UDP
  if (udp.begin(UDP_PORT)) {
    Serial.print("UDP Server iniciado na porta: ");
    Serial.println(UDP_PORT);
  } else {
    Serial.println("Falha ao iniciar UDP Server!");
    return;
  }
  
  // Configurar rotas do servidor web
  setupWebRoutes();
  
  // Iniciar servidor web
  webServer.begin();
  Serial.print("Web Server iniciado na porta: ");
  Serial.println(WEB_PORT);
  
  Serial.println("EasyWay Server - Sistema inicializado com sucesso!");
  Serial.print("Acesse: http://");
  Serial.println(IP);
  
  digitalWrite(LED_STATUS_PIN, HIGH);
}

void loop() {
  // Processar mensagens recebidas via UDP
  processUDPMessages();
  
  // Processar requisições web
  webServer.handleClient();
  
  // Verificar timeout do cliente
  checkClientTimeout();
  
  delay(100);
}

void processUDPMessages() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    
    Serial.print("Server received from ");
    Serial.print(udp.remoteIP());
    Serial.print(":");
    Serial.print(udp.remotePort());
    Serial.print(" - ");
    Serial.println(incomingPacket);
    
    // Processar mensagem JSON
    processMessage(String(incomingPacket));
    
    // Enviar resposta simples
    String response = "ACK: " + String(incomingPacket);
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print(response);
    udp.endPacket();
    Serial.print("Server sent: ");
    Serial.println(response);
    
    lastHeartbeat = millis();
  }
}

void processMessage(String message) {
  // Tentar parsear JSON
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.print("Erro ao parsear JSON: ");
    Serial.println(error.c_str());
    return;
  }
  
  String deviceId = doc["id"];
  String type = doc["type"];
  String status = doc["status"];
  
  if (type == "status" && status == "online") {
    clientStatus = "Conectado (" + deviceId + ")";
    Serial.println("Cliente conectado: " + deviceId);
  } else if (type == "heartbeat") {
    clientStatus = "Ativo (" + deviceId + ")";
    Serial.println("Heartbeat recebido de: " + deviceId);
  } else if (type == "status") {
    clientStatus = status + " (" + deviceId + ")";
    Serial.println("Status atualizado: " + deviceId + " = " + status);
  } else if (type == "sensor_data") {
    // Processar dados do sensor BME280
    lastTemperature = doc["temperature"];
    lastHumidity = doc["humidity"];
    lastPressure = doc["pressure"];
    lastAltitude = doc["altitude"];
    lastSensorUpdate = millis();
    sensorStatus = "Atualizado";
    
    Serial.println("=== Dados BME280 Recebidos ===");
    Serial.print("Temperatura: ");
    Serial.print(lastTemperature, 2);
    Serial.println(" °C");
    Serial.print("Umidade: ");
    Serial.print(lastHumidity, 2);
    Serial.println(" %");
    Serial.print("Pressão: ");
    Serial.print(lastPressure, 2);
    Serial.println(" hPa");
    Serial.print("Altitude: ");
    Serial.print(lastAltitude, 2);
    Serial.println(" m");
    Serial.println("=============================");
    
    clientStatus = "Ativo com dados (" + deviceId + ")";
  }
  
  lastMessage = message;
}

void checkClientTimeout() {
  if (millis() - lastHeartbeat > 60000) { // 1 minuto sem heartbeat
    clientStatus = "Timeout";
  }
}

void setupWebRoutes() {
  // Página principal
  webServer.on("/", handleRoot);
  
  // API endpoints
  webServer.on("/api/status", HTTP_GET, handleStatus);
  webServer.on("/api/devices", HTTP_GET, handleDevices);
  webServer.on("/api/sensor", HTTP_GET, handleSensorData);
  webServer.on("/api/command", HTTP_POST, handleCommand);
  
  // Página 404
  webServer.onNotFound(handleNotFound);
}

void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html><head>";
  html += "<title>EasyWay Server</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }";
  html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; }";
  html += ".status { background: #e8f5e8; padding: 15px; border-radius: 5px; margin: 10px 0; }";
  html += ".message { background: #f0f8ff; padding: 15px; border-radius: 5px; margin: 10px 0; }";
  html += "button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; margin: 5px; }";
  html += "button:hover { background: #45a049; }";
  html += ".danger { background: #f44336; }";
  html += ".danger:hover { background: #da190b; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>🛰️ EasyWay Server</h1>";
  html += "<div class='status'>";
  html += "<h3>Status do Sistema</h3>";
  html += "<p><strong>Cliente:</strong> " + clientStatus + "</p>";
  html += "<p><strong>Wi-Fi:</strong> " + String(WIFI_SSID) + "</p>";
  html += "<p><strong>IP:</strong> " + WiFi.softAPIP().toString() + "</p>";
  html += "<p><strong>UDP Port:</strong> " + String(UDP_PORT) + "</p>";
  html += "</div>";
  html += "<div class='message'>";
  html += "<h3>Última Mensagem</h3>";
  html += "<p>" + lastMessage + "</p>";
  html += "</div>";
  
  // Seção de dados do sensor BME280
  html += "<div class='status'>";
  html += "<h3>🌡️ Dados do Sensor BME280</h3>";
  html += "<p><strong>Status:</strong> " + sensorStatus + "</p>";
  html += "<p><strong>Temperatura:</strong> " + String(lastTemperature, 2) + " °C</p>";
  html += "<p><strong>Umidade:</strong> " + String(lastHumidity, 2) + " %</p>";
  html += "<p><strong>Pressão:</strong> " + String(lastPressure, 2) + " hPa</p>";
  html += "<p><strong>Altitude:</strong> " + String(lastAltitude, 2) + " m</p>";
  html += "<p><strong>Última atualização:</strong> " + String((millis() - lastSensorUpdate) / 1000) + "s atrás</p>";
  html += "</div>";
  
  html += "<h3>Controle de Dispositivos</h3>";
  html += "<button onclick='sendCommand(\"abrir\")'>🔓 Abrir Porta</button>";
  html += "<button onclick='sendCommand(\"fechar\")' class='danger'>🔒 Fechar Porta</button>";
  html += "<button onclick='sendCommand(\"status\")'>📊 Solicitar Status</button>";
  html += "<button onclick='sendCommand(\"temperatura\")'>🌡️ Solicitar Dados do Sensor</button>";
  html += "<button onclick='location.reload()'>🔄 Atualizar</button>";
  html += "<script>";
  html += "function sendCommand(cmd) {";
  html += "  fetch('/api/command', {";
  html += "    method: 'POST',";
  html += "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },";
  html += "    body: 'command=' + cmd";
  html += "  }).then(response => response.text())";
  html += "    .then(data => {";
  html += "      alert('Comando enviado: ' + cmd);";
  html += "      setTimeout(() => location.reload(), 1000);";
  html += "    });";
  html += "}";
  html += "</script>";
  html += "</div></body></html>";
  
  webServer.send(200, "text/html", html);
}

void handleStatus() {
  DynamicJsonDocument doc(1024);
  doc["status"] = "online";
  doc["client_status"] = clientStatus;
  doc["last_message"] = lastMessage;
  doc["uptime"] = millis();
  doc["wifi_ssid"] = WIFI_SSID;
  doc["ip"] = WiFi.softAPIP().toString();
  
  // Adicionar dados do sensor
  doc["sensor"]["temperature"] = lastTemperature;
  doc["sensor"]["humidity"] = lastHumidity;
  doc["sensor"]["pressure"] = lastPressure;
  doc["sensor"]["altitude"] = lastAltitude;
  doc["sensor"]["status"] = sensorStatus;
  doc["sensor"]["last_update"] = lastSensorUpdate;
  
  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleSensorData() {
  DynamicJsonDocument doc(512);
  doc["temperature"] = lastTemperature;
  doc["humidity"] = lastHumidity;
  doc["pressure"] = lastPressure;
  doc["altitude"] = lastAltitude;
  doc["status"] = sensorStatus;
  doc["last_update"] = lastSensorUpdate;
  doc["age_seconds"] = (millis() - lastSensorUpdate) / 1000;
  
  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleDevices() {
  DynamicJsonDocument doc(1024);
  JsonArray devices = doc.createNestedArray("devices");
  
  JsonObject device = devices.createNestedObject();
  device["id"] = "cliente01";
  device["status"] = clientStatus;
  device["last_seen"] = lastHeartbeat;
  device["ip"] = "192.168.4.2";
  
  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleCommand() {
  if (webServer.hasArg("command")) {
    String command = webServer.arg("command");
    String message = "{\"id\":\"servidor\",\"type\":\"command\",\"cmd\":\"" + command + "\"}";
    
    // Enviar comando via UDP para o cliente
    IPAddress clientIP(192, 168, 4, 2); // IP padrão do cliente
    udp.beginPacket(clientIP, UDP_PORT);
    udp.print(message);
    udp.endPacket();
    
    Serial.println("Comando enviado: " + command);
    webServer.send(200, "text/plain", "Comando enviado: " + command);
  } else {
    webServer.send(400, "text/plain", "Comando não especificado");
  }
}

void handleNotFound() {
  String message = "Página não encontrada\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMétodo: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArgumentos: ";
  message += webServer.args();
  message += "\n";
  
  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  
  webServer.send(404, "text/plain", message);
}