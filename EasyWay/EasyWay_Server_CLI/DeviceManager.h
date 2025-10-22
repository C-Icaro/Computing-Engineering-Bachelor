#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

struct Device {
    String id;
    String status;
    String lastCommand;
    unsigned long lastSeen;
    bool isOnline;
    String lastMessage;
    IPAddress clientIP;  // IP do cliente conectado
};

struct LogEntry {
    String deviceId;
    String command;
    unsigned long timestamp;
    String details;
};

class DeviceManager {
private:
    static const int MAX_DEVICES = 10;
    static const int MAX_LOGS = 100;
    
    Device devices[MAX_DEVICES];
    LogEntry logs[MAX_LOGS];
    int deviceCount;
    int logCount;
    int logIndex;
    
public:
    DeviceManager();
    ~DeviceManager();
    
    // Inicialização
    void initialize();
    
    // Gerenciamento de dispositivos
    bool addDevice(const String& id);
    bool addDevice(const String& id, const IPAddress& clientIP);
    bool removeDevice(const String& id);
    Device* getDevice(const String& id);
    void updateDeviceStatus(const String& id, const String& status);
    void updateDeviceStatus(const String& id, const String& status, unsigned long timestamp);
    void updateDeviceIP(const String& id, const IPAddress& clientIP);
    IPAddress getDeviceIP(const String& id);
    
    // Controle de dispositivos
    bool executeCommand(const String& id, const String& command);
    String getDeviceStatus(const String& id);
    bool isDeviceOnline(const String& id);
    
    // Mensagens
    void addMessage(const String& deviceId, const String& message);
    String getLastMessage(const String& deviceId);
    
    // Logs
    void addLog(const String& deviceId, const String& command, unsigned long timestamp);
    void addLog(const String& deviceId, const String& command, unsigned long timestamp, const String& details);
    String getLogsJson();
    void clearLogs();
    
    // Status geral
    String getDevicesJson();
    int getOnlineDeviceCount();
    int getTotalDeviceCount();
    void updateDeviceStatus();
    
    // Utilitários
    void printDeviceList();
    void printLogs();
    String formatTimestamp(unsigned long timestamp);
    
private:
    int findDeviceIndex(const String& id);
    void shiftLogs();
};

#endif
