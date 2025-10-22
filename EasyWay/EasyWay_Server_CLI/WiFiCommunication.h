#ifndef WIFI_COMMUNICATION_H
#define WIFI_COMMUNICATION_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

class WiFiCommunication {
private:
    WiFiUdp udp;
    bool initialized;
    String ssid;
    String password;
    int serverPort;
    IPAddress serverIP;
    
    // Buffer para mensagens
    char messageBuffer[512];
    
public:
    WiFiCommunication();
    ~WiFiCommunication();
    
    // Inicialização
    bool initialize(const char* ssid, const char* password, int port);
    void shutdown();
    
    // Comunicação
    bool sendMessage(const String& deviceId, const String& message);
    bool hasMessage();
    String receiveMessage();
    
    // Configuração
    void setServerIP(const IPAddress& ip);
    IPAddress getServerIP() const;
    int getServerPort() const;
    
    // Status
    bool isInitialized() const;
    bool isConnected() const;
    void printNetworkInfo();
    
    // Utilitários
    String getDeviceIP() const;
    String getDeviceMAC() const;
    int getSignalStrength() const;
    
private:
    bool connectToNetwork();
    void startUDPServer();
    String extractDeviceIdFromMessage(const String& message);
};

#endif
