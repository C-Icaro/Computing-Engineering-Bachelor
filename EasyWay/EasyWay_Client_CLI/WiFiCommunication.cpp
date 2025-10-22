#include "WiFiCommunication.h"

WiFiCommunication::WiFiCommunication() : initialized(false), serverPort(8080) {
    serverIP = IPAddress(192, 168, 4, 1); // IP padrão do servidor
}

WiFiCommunication::~WiFiCommunication() {
    shutdown();
}

bool WiFiCommunication::initialize(const char* ssid, const char* password, int port) {
    this->ssid = String(ssid);
    this->password = String(password);
    this->serverPort = port;
    
    // Configurar modo WiFi
    WiFi.mode(WIFI_STA);
    
    // Conectar à rede
    if (!connectToNetwork()) {
        Serial.println("ERRO: Falha ao conectar à rede WiFi");
        return false;
    }
    
    // Iniciar cliente UDP
    startUDPClient();
    
    initialized = true;
    Serial.println("Comunicação WiFi inicializada com sucesso");
    printNetworkInfo();
    
    return true;
}

void WiFiCommunication::shutdown() {
    if (initialized) {
        udp.stop();
        WiFi.disconnect();
        initialized = false;
        Serial.println("Comunicação WiFi desligada");
    }
}

bool WiFiCommunication::sendMessage(const String& deviceId, const String& message) {
    if (!initialized) {
        Serial.println("ERRO: WiFi não inicializado");
        return false;
    }
    
    // Criar pacote UDP com ID do dispositivo e mensagem
    String packet = deviceId + "|" + message;
    
    // Enviar via UDP
    udp.beginPacket(serverIP, serverPort);
    udp.write((uint8_t*)packet.c_str(), packet.length());
    bool success = udp.endPacket();
    
    if (success) {
        Serial.print("Mensagem enviada para ");
        Serial.print(deviceId);
        Serial.print(": ");
        Serial.println(message);
    } else {
        Serial.println("ERRO: Falha ao enviar mensagem WiFi");
    }
    
    return success;
}

bool WiFiCommunication::hasMessage() {
    if (!initialized) {
        return false;
    }
    
    int packetSize = udp.parsePacket();
    return packetSize > 0;
}

String WiFiCommunication::receiveMessage() {
    if (!initialized || !hasMessage()) {
        return "";
    }
    
    // Ler dados do pacote UDP
    int len = udp.read(messageBuffer, sizeof(messageBuffer) - 1);
    if (len > 0) {
        messageBuffer[len] = '\0';
        
        String fullMessage = String(messageBuffer);
        Serial.println("=== MENSAGEM UDP RECEBIDA ===");
        Serial.print("Tamanho: ");
        Serial.println(len);
        Serial.print("Conteúdo: ");
        Serial.println(fullMessage);
        Serial.println("=== FIM DA MENSAGEM UDP ===");
        
        return fullMessage;
    }
    
    Serial.println("ERRO: Nenhum dado recebido do UDP");
    return "";
}

void WiFiCommunication::setServerIP(const IPAddress& ip) {
    serverIP = ip;
    Serial.print("IP do servidor alterado para: ");
    Serial.println(serverIP);
}

IPAddress WiFiCommunication::getServerIP() const {
    return serverIP;
}

int WiFiCommunication::getServerPort() const {
    return serverPort;
}

bool WiFiCommunication::isInitialized() const {
    return initialized;
}

bool WiFiCommunication::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiCommunication::printNetworkInfo() {
    if (initialized) {
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("IP do dispositivo: ");
        Serial.println(WiFi.localIP());
        Serial.print("IP do servidor: ");
        Serial.println(serverIP);
        Serial.print("Porta: ");
        Serial.println(serverPort);
        Serial.print("Endereço MAC: ");
        Serial.println(WiFi.macAddress());
        Serial.print("Força do sinal: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    }
}

bool WiFiCommunication::connectToNetwork() {
    Serial.print("Conectando à rede WiFi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Aguardar conexão (timeout de 20 segundos)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("Conectado à rede WiFi!");
        return true;
    } else {
        Serial.println();
        Serial.println("ERRO: Timeout ao conectar à rede WiFi");
        return false;
    }
}

void WiFiCommunication::startUDPClient() {
    if (udp.begin(serverPort)) {
        Serial.print("Cliente UDP iniciado na porta ");
        Serial.println(serverPort);
    } else {
        Serial.println("ERRO: Falha ao iniciar cliente UDP");
    }
}

String WiFiCommunication::extractDeviceIdFromMessage(const String& message) {
    int separatorIndex = message.indexOf('|');
    if (separatorIndex > 0) {
        return message.substring(0, separatorIndex);
    }
    return "";
}
