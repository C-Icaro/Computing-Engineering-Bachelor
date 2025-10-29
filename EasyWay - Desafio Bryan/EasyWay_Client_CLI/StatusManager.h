#ifndef STATUS_MANAGER_H
#define STATUS_MANAGER_H

#include <Arduino.h>

// Forward declaration para evitar dependência circular
class DeviceController;

class StatusManager {
private:
    bool initialized;
    bool connected;
    String lastError;
    unsigned long lastStatusUpdate;
    const unsigned long STATUS_UPDATE_INTERVAL = 1000; // 1 segundo
    
    // Referência para controlar LEDs
    DeviceController* deviceController;
    
public:
    StatusManager();
    ~StatusManager();
    
    // Inicialização
    void initialize();
    void initialize(DeviceController* controller);
    void shutdown();
    
    // Status
    void setConnected(bool state);
    void setError(const String& error);
    void clearError();
    
    // Atualização
    void updateStatus();
    
    // Getters
    bool isConnected() const;
    bool hasError() const;
    String getLastError() const;
    bool isInitialized() const;
    
    // Utilitários
    void printStatus();
    String getStatusString() const;
    
private:
    void updateLEDs();
    void logStatusChange(const String& status);
};

#endif
