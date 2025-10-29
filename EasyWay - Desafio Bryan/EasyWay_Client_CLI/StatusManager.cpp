#include "StatusManager.h"
#include "DeviceController.h"

StatusManager::StatusManager() : initialized(false), connected(false), lastStatusUpdate(0), deviceController(nullptr) {
}

StatusManager::~StatusManager() {
    shutdown();
}

void StatusManager::initialize() {
    initialized = true;
    Serial.println("Gerenciador de status inicializado");
}

void StatusManager::initialize(DeviceController* controller) {
    deviceController = controller;
    initialized = true;
    Serial.println("Gerenciador de status inicializado com DeviceController");
}

void StatusManager::shutdown() {
    if (initialized) {
        initialized = false;
        Serial.println("Gerenciador de status desligado");
    }
}

void StatusManager::setConnected(bool state) {
    if (connected != state) {
        connected = state;
        logStatusChange(state ? "Conectado" : "Desconectado");
        Serial.print("StatusManager: setConnected(");
        Serial.print(state ? "true" : "false");
        Serial.println(")");
    }
}

void StatusManager::setError(const String& error) {
    lastError = error;
    Serial.println("ERRO: " + error);
}

void StatusManager::clearError() {
    if (lastError.length() > 0) {
        Serial.println("Erro limpo: " + lastError);
        lastError = "";
    }
}

void StatusManager::updateStatus() {
    if (!initialized) return;
    
    if (millis() - lastStatusUpdate > STATUS_UPDATE_INTERVAL) {
        updateLEDs();
        lastStatusUpdate = millis();
    }
}

bool StatusManager::isConnected() const {
    return connected;
}

bool StatusManager::hasError() const {
    return lastError.length() > 0;
}

String StatusManager::getLastError() const {
    return lastError;
}

bool StatusManager::isInitialized() const {
    return initialized;
}

void StatusManager::printStatus() {
    Serial.println("=== Status do Sistema ===");
    Serial.print("Conectado: ");
    Serial.println(connected ? "SIM" : "NÃO");
    Serial.print("Erro: ");
    Serial.println(hasError() ? lastError : "Nenhum");
    Serial.print("Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" segundos");
    Serial.println("========================");
}

String StatusManager::getStatusString() const {
    String status = "Status: ";
    status += connected ? "Conectado" : "Desconectado";
    
    if (hasError()) {
        status += " | Erro: " + lastError;
    }
    
    return status;
}

void StatusManager::updateLEDs() {
    if (deviceController == nullptr) {
        Serial.println("StatusManager: deviceController é nullptr!");
        return;
    }
    
    // Controlar LEDs baseado no status
    if (connected && !hasError()) {
        deviceController->setConnectedLED(true);
        deviceController->setErrorLED(false);
        Serial.println("StatusManager: LEDs - Verde ON, Vermelho OFF");
    } else {
        deviceController->setConnectedLED(false);
        deviceController->setErrorLED(true);
        Serial.println("StatusManager: LEDs - Verde OFF, Vermelho ON");
    }
    
    // Log do status a cada 10 segundos
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 10000) {
        Serial.println(getStatusString());
        lastLog = millis();
    }
}

void StatusManager::logStatusChange(const String& status) {
    Serial.print("[");
    Serial.print(millis() / 1000);
    Serial.print("s] Status alterado: ");
    Serial.println(status);
}
