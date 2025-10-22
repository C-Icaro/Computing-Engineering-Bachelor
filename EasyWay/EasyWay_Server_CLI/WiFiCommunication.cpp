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
    
    // Configurar modo WiFi como Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    
    Serial.println("Access Point criado: " + this->ssid);
    
    // Iniciar servidor UDP
    startUDPServer();
    
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
    
    Serial.println("=== ENVIANDO MENSAGEM UDP ===");
    Serial.print("Device ID: ");
    Serial.println(deviceId);
    Serial.print("Mensagem: ");
    Serial.println(message);
    Serial.print("Pacote completo: ");
    Serial.println(packet);
    Serial.print("IP de destino: ");
    Serial.println(serverIP);
    Serial.print("Porta: ");
    Serial.println(serverPort);
    
    // Enviar via UDP
    udp.beginPacket(serverIP, serverPort);
    udp.write((uint8_t*)packet.c_str(), packet.length());
    bool success = udp.endPacket();
    
    if (success) {
        Serial.println("Mensagem UDP enviada com sucesso");
    } else {
        Serial.println("ERRO: Falha ao enviar mensagem UDP");
    }
    Serial.println("=== FIM DO ENVIO ===");
    
    return success;
}

bool WiFiCommunication::sendMessageToIP(const IPAddress& targetIP, const String& deviceId, const String& message) {
    if (!initialized) {
        Serial.println("ERRO: WiFi não inicializado");
        return false;
    }
    
    // Criar pacote UDP com ID do dispositivo e mensagem
    String packet = deviceId + "|" + message;
    
    Serial.println("=== ENVIANDO MENSAGEM UDP PARA IP ESPECÍFICO ===");
    Serial.print("Device ID: ");
    Serial.println(deviceId);
    Serial.print("Mensagem: ");
    Serial.println(message);
    Serial.print("Pacote completo: ");
    Serial.println(packet);
    Serial.print("IP de destino: ");
    Serial.println(targetIP);
    Serial.print("Porta: ");
    Serial.println(serverPort);
    
    // Enviar via UDP para o IP específico
    udp.beginPacket(targetIP, serverPort);
    udp.write((uint8_t*)packet.c_str(), packet.length());
    bool success = udp.endPacket();
    
    if (success) {
        Serial.println("Mensagem UDP enviada com sucesso para IP específico");
    } else {
        Serial.println("ERRO: Falha ao enviar mensagem UDP para IP específico");
    }
    Serial.println("=== FIM DO ENVIO ===");
    
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
        Serial.print("Mensagem WiFi recebida: ");
        Serial.println(fullMessage);
        
        // Extrair mensagem JSON do formato "deviceId|message"
        int separatorIndex = fullMessage.indexOf('|');
        if (separatorIndex > 0) {
            String jsonMessage = fullMessage.substring(separatorIndex + 1);
            Serial.print("Mensagem JSON extraída: ");
            Serial.println(jsonMessage);
            return jsonMessage;
        } else {
            Serial.println("ERRO: Formato de mensagem inválido");
            return "";
        }
    }
    
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
    return WiFi.getMode() == WIFI_AP;
}

void WiFiCommunication::printNetworkInfo() {
    if (initialized) {
        Serial.print("SSID do Access Point: ");
        Serial.println(ssid);
        Serial.print("IP do Access Point: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("IP do servidor: ");
        Serial.println(serverIP);
        Serial.print("Porta: ");
        Serial.println(serverPort);
        Serial.print("Endereço MAC: ");
        Serial.println(WiFi.softAPmacAddress());
        Serial.print("Número de clientes conectados: ");
        Serial.println(WiFi.softAPgetStationNum());
    }
}

IPAddress WiFiCommunication::getRemoteIP() {
    if (!initialized) {
        return IPAddress(0, 0, 0, 0);
    }
    
    // Obter IP do último pacote UDP recebido
    return udp.remoteIP();
}

String WiFiCommunication::getDeviceIP() const {
    return WiFi.softAPIP().toString();
}

String WiFiCommunication::getDeviceMAC() const {
    return WiFi.softAPmacAddress();
}

int WiFiCommunication::getSignalStrength() const {
    return WiFi.RSSI();
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

void WiFiCommunication::startUDPServer() {
    if (udp.begin(serverPort)) {
        Serial.print("Servidor UDP iniciado na porta ");
        Serial.println(serverPort);
    } else {
        Serial.println("ERRO: Falha ao iniciar servidor UDP");
    }
}

String WiFiCommunication::extractDeviceIdFromMessage(const String& message) {
    int separatorIndex = message.indexOf('|');
    if (separatorIndex > 0) {
        return message.substring(0, separatorIndex);
    }
    return "";
}
