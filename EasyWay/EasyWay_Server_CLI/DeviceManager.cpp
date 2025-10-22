#include "DeviceManager.h"

DeviceManager::DeviceManager() : deviceCount(0), logCount(0), logIndex(0) {
    // Inicializar arrays
    for (int i = 0; i < MAX_DEVICES; i++) {
        devices[i].id = "";
        devices[i].status = "offline";
        devices[i].lastCommand = "";
        devices[i].lastSeen = 0;
        devices[i].isOnline = false;
        devices[i].lastMessage = "";
        devices[i].clientIP = IPAddress(0, 0, 0, 0);
    }
    
    for (int i = 0; i < MAX_LOGS; i++) {
        logs[i].deviceId = "";
        logs[i].command = "";
        logs[i].timestamp = 0;
        logs[i].details = "";
    }
}

DeviceManager::~DeviceManager() {
}

void DeviceManager::initialize() {
    Serial.println("Gerenciador de dispositivos inicializado");
}

bool DeviceManager::addDevice(const String& id) {
    return addDevice(id, IPAddress(0, 0, 0, 0));
}

bool DeviceManager::addDevice(const String& id, const IPAddress& clientIP) {
    if (deviceCount >= MAX_DEVICES) {
        Serial.println("ERRO: Máximo de dispositivos atingido");
        return false;
    }
    
    int index = findDeviceIndex(id);
    if (index != -1) {
        Serial.println("Dispositivo já existe: " + id);
        // Atualizar IP se fornecido
        if (clientIP != IPAddress(0, 0, 0, 0)) {
            devices[index].clientIP = clientIP;
            Serial.print("IP do dispositivo atualizado: ");
            Serial.println(clientIP);
        }
        return true;
    }
    
    devices[deviceCount].id = id;
    devices[deviceCount].status = "offline";
    devices[deviceCount].lastCommand = "";
    devices[deviceCount].lastSeen = millis();
    devices[deviceCount].isOnline = false;
    devices[deviceCount].lastMessage = "";
    devices[deviceCount].clientIP = clientIP;
    
    deviceCount++;
    
    Serial.println("Dispositivo adicionado: " + id);
    if (clientIP != IPAddress(0, 0, 0, 0)) {
        Serial.print("IP do cliente: ");
        Serial.println(clientIP);
    }
    addLog(id, "device_added", millis(), "Dispositivo registrado no sistema");
    
    return true;
}

bool DeviceManager::removeDevice(const String& id) {
    int index = findDeviceIndex(id);
    if (index == -1) {
        Serial.println("Dispositivo não encontrado: " + id);
        return false;
    }
    
    // Remover dispositivo (mover todos os elementos uma posição para trás)
    for (int i = index; i < deviceCount - 1; i++) {
        devices[i] = devices[i + 1];
    }
    
    deviceCount--;
    
    Serial.println("Dispositivo removido: " + id);
    addLog(id, "device_removed", millis(), "Dispositivo removido do sistema");
    
    return true;
}

Device* DeviceManager::getDevice(const String& id) {
    int index = findDeviceIndex(id);
    if (index == -1) {
        return nullptr;
    }
    return &devices[index];
}

void DeviceManager::updateDeviceStatus(const String& id, const String& status) {
    updateDeviceStatus(id, status, millis());
}

void DeviceManager::updateDeviceStatus(const String& id, const String& status, unsigned long timestamp) {
    Device* device = getDevice(id);
    if (device == nullptr) {
        // Adicionar dispositivo se não existir
        addDevice(id);
        device = getDevice(id);
    }
    
    if (device != nullptr) {
        device->status = status;
        device->lastSeen = timestamp;
        device->isOnline = (millis() - timestamp) < 60000; // Online se visto nos últimos 60 segundos
        
        Serial.print("Status atualizado - ");
        Serial.print(id);
        Serial.print(": ");
        Serial.println(status);
    }
}

void DeviceManager::updateDeviceIP(const String& id, const IPAddress& clientIP) {
    Device* device = getDevice(id);
    if (device == nullptr) {
        // Adicionar dispositivo se não existir
        addDevice(id, clientIP);
        return;
    }
    
    device->clientIP = clientIP;
    Serial.print("IP atualizado para dispositivo ");
    Serial.print(id);
    Serial.print(": ");
    Serial.println(clientIP);
}

IPAddress DeviceManager::getDeviceIP(const String& id) {
    Device* device = getDevice(id);
    if (device == nullptr) {
        return IPAddress(0, 0, 0, 0);
    }
    return device->clientIP;
}

bool DeviceManager::executeCommand(const String& id, const String& command) {
    Device* device = getDevice(id);
    if (device == nullptr) {
        Serial.println("ERRO: Dispositivo não encontrado: " + id);
        return false;
    }
    
    device->lastCommand = command;
    device->lastSeen = millis();
    device->isOnline = true;
    
    Serial.print("Comando executado - ");
    Serial.print(id);
    Serial.print(": ");
    Serial.println(command);
    
    addLog(id, command, millis(), "Comando enviado via interface web");
    
    return true;
}

String DeviceManager::getDeviceStatus(const String& id) {
    Device* device = getDevice(id);
    if (device == nullptr) {
        return "not_found";
    }
    return device->status;
}

bool DeviceManager::isDeviceOnline(const String& id) {
    Device* device = getDevice(id);
    if (device == nullptr) {
        return false;
    }
    return device->isOnline;
}

void DeviceManager::addMessage(const String& deviceId, const String& message) {
    Device* device = getDevice(deviceId);
    if (device != nullptr) {
        device->lastMessage = message;
        device->lastSeen = millis();
        device->isOnline = true;
        
        Serial.print("Mensagem recebida de ");
        Serial.print(deviceId);
        Serial.print(": ");
        Serial.println(message);
        
        addLog(deviceId, "message_received", millis(), message);
    }
}

String DeviceManager::getLastMessage(const String& deviceId) {
    Device* device = getDevice(deviceId);
    if (device == nullptr) {
        return "";
    }
    return device->lastMessage;
}

void DeviceManager::addLog(const String& deviceId, const String& command, unsigned long timestamp) {
    addLog(deviceId, command, timestamp, "");
}

void DeviceManager::addLog(const String& deviceId, const String& command, unsigned long timestamp, const String& details) {
    logs[logIndex].deviceId = deviceId;
    logs[logIndex].command = command;
    logs[logIndex].timestamp = timestamp;
    logs[logIndex].details = details;
    
    logIndex = (logIndex + 1) % MAX_LOGS;
    if (logCount < MAX_LOGS) {
        logCount++;
    }
}

String DeviceManager::getLogsJson() {
    DynamicJsonDocument doc(2048);
    JsonArray logsArray = doc.to<JsonArray>();
    
    int startIndex = (logCount == MAX_LOGS) ? logIndex : 0;
    int count = logCount;
    
    for (int i = 0; i < count; i++) {
        int index = (startIndex + i) % MAX_LOGS;
        
        JsonObject logEntry = logsArray.createNestedObject();
        logEntry["deviceId"] = logs[index].deviceId;
        logEntry["command"] = logs[index].command;
        logEntry["timestamp"] = logs[index].timestamp;
        logEntry["formattedTime"] = formatTimestamp(logs[index].timestamp);
        logEntry["details"] = logs[index].details;
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

void DeviceManager::clearLogs() {
    logCount = 0;
    logIndex = 0;
    Serial.println("Logs limpos");
}

String DeviceManager::getDevicesJson() {
    DynamicJsonDocument doc(1024);
    JsonArray devicesArray = doc.to<JsonArray>();
    
    for (int i = 0; i < deviceCount; i++) {
        JsonObject device = devicesArray.createNestedObject();
        device["id"] = devices[i].id;
        device["status"] = devices[i].status;
        device["lastCommand"] = devices[i].lastCommand;
        device["lastSeen"] = devices[i].lastSeen;
        device["isOnline"] = devices[i].isOnline;
        device["lastMessage"] = devices[i].lastMessage;
        device["clientIP"] = devices[i].clientIP.toString();
        device["formattedLastSeen"] = formatTimestamp(devices[i].lastSeen);
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

int DeviceManager::getOnlineDeviceCount() {
    int count = 0;
    for (int i = 0; i < deviceCount; i++) {
        if (devices[i].isOnline) {
            count++;
        }
    }
    return count;
}

int DeviceManager::getTotalDeviceCount() {
    return deviceCount;
}

void DeviceManager::updateDeviceStatus() {
    unsigned long currentTime = millis();
    for (int i = 0; i < deviceCount; i++) {
        if (currentTime - devices[i].lastSeen > 60000) { // 60 segundos
            devices[i].isOnline = false;
            devices[i].status = "offline";
        }
    }
}

void DeviceManager::printDeviceList() {
    Serial.println("=== Lista de Dispositivos ===");
    for (int i = 0; i < deviceCount; i++) {
        Serial.print("ID: ");
        Serial.print(devices[i].id);
        Serial.print(" | Status: ");
        Serial.print(devices[i].status);
        Serial.print(" | Online: ");
        Serial.print(devices[i].isOnline ? "Sim" : "Não");
        Serial.print(" | Último comando: ");
        Serial.println(devices[i].lastCommand);
    }
    Serial.println("=============================");
}

void DeviceManager::printLogs() {
    Serial.println("=== Logs do Sistema ===");
    int startIndex = (logCount == MAX_LOGS) ? logIndex : 0;
    int count = logCount;
    
    for (int i = 0; i < count; i++) {
        int index = (startIndex + i) % MAX_LOGS;
        Serial.print(formatTimestamp(logs[index].timestamp));
        Serial.print(" | ");
        Serial.print(logs[index].deviceId);
        Serial.print(" | ");
        Serial.print(logs[index].command);
        if (logs[index].details.length() > 0) {
            Serial.print(" | ");
            Serial.print(logs[index].details);
        }
        Serial.println();
    }
    Serial.println("=======================");
}

String DeviceManager::formatTimestamp(unsigned long timestamp) {
    unsigned long seconds = timestamp / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    seconds = seconds % 60;
    minutes = minutes % 60;
    hours = hours % 24;
    
    String result = "";
    if (hours < 10) result += "0";
    result += String(hours) + ":";
    if (minutes < 10) result += "0";
    result += String(minutes) + ":";
    if (seconds < 10) result += "0";
    result += String(seconds);
    
    return result;
}

int DeviceManager::findDeviceIndex(const String& id) {
    for (int i = 0; i < deviceCount; i++) {
        if (devices[i].id == id) {
            return i;
        }
    }
    return -1;
}

void DeviceManager::shiftLogs() {
    // Implementação para quando necessário reorganizar logs
    // Por enquanto, usando buffer circular
}
