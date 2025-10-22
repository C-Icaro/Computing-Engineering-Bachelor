#include "DeviceController.h"

DeviceController::DeviceController() : initialized(false), doorOpen(false), lastTransmission(0) {
}

DeviceController::~DeviceController() {
    shutdown();
}

void DeviceController::initialize() {
    // Configurar pinos SEGUROS
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
    
    initialized = true;
    Serial.println("Controlador de dispositivos inicializado");
    printPinConfiguration();
}

void DeviceController::shutdown() {
    if (initialized) {
        // Desligar todos os dispositivos
        digitalWrite(DOOR_RELAY_PIN, LOW);
        digitalWrite(LED_CONNECTED_PIN, LOW);
        digitalWrite(LED_ERROR_PIN, LOW);
        digitalWrite(LED_TRANSMIT_PIN, LOW);
        
        initialized = false;
        Serial.println("Controlador de dispositivos desligado");
    }
}

void DeviceController::openDoor() {
    if (!initialized) {
        Serial.println("ERRO: DeviceController não inicializado - não é possível abrir a porta");
        return;
    }
    
    Serial.println("=== ABRINDO PORTA ===");
    Serial.println("Ativando relé (GPIO" + String(DOOR_RELAY_PIN) + ")");
    
    digitalWrite(DOOR_RELAY_PIN, HIGH);
    doorOpen = true;
    Serial.println("Porta ABERTA (relé ativado)");
    
    // Manter aberto por 1 segundo
    Serial.println("Aguardando 1 segundo...");
    delay(1000);
    
    digitalWrite(DOOR_RELAY_PIN, LOW);
    doorOpen = false;
    Serial.println("Porta FECHADA (relé desativado)");
    Serial.println("=== FIM DA ABERTURA ===");
}

void DeviceController::closeDoor() {
    if (!initialized) return;
    
    digitalWrite(DOOR_RELAY_PIN, LOW);
    doorOpen = false;
    Serial.println("Porta FECHADA");
}

String DeviceController::getDoorStatus() const {
    return doorOpen ? "aberto" : "fechado";
}

void DeviceController::checkLocalCommands() {
    if (!initialized) return;
    
    if (digitalRead(BUTTON_PIN) == LOW) {
        debounceButton();
        
        if (digitalRead(BUTTON_PIN) == LOW) {
            handleButtonPress();
            
            // Aguardar soltar o botão
            while (digitalRead(BUTTON_PIN) == LOW) {
                delay(10);
            }
            Serial.println("Botão solto");
        }
    }
}

void DeviceController::setConnectedLED(bool state) {
    if (!initialized) return;
    
    digitalWrite(LED_CONNECTED_PIN, state ? HIGH : LOW);
    Serial.print("LED Verde (Conectado): ");
    Serial.println(state ? "LIGADO" : "APAGADO");
}

void DeviceController::setErrorLED(bool state) {
    if (!initialized) return;
    
    digitalWrite(LED_ERROR_PIN, state ? HIGH : LOW);
    Serial.print("LED Vermelho (Erro): ");
    Serial.println(state ? "LIGADO" : "APAGADO");
}

void DeviceController::setTransmitLED(bool state) {
    if (!initialized) return;
    
    digitalWrite(LED_TRANSMIT_PIN, state ? HIGH : LOW);
    lastTransmission = millis();
    Serial.print("LED Amarelo (Transmitindo): ");
    Serial.println(state ? "LIGADO" : "APAGADO");
}

void DeviceController::updateLEDs() {
    if (!initialized) return;
    
    // Desligar LED de transmissão após tempo determinado
    if (digitalRead(LED_TRANSMIT_PIN) == HIGH && 
        millis() - lastTransmission > TRANSMISSION_LED_TIME) {
        digitalWrite(LED_TRANSMIT_PIN, LOW);
        Serial.println("LED Amarelo: APAGADO (transmissão concluída)");
    }
}

bool DeviceController::isInitialized() const {
    return initialized;
}

void DeviceController::printPinConfiguration() {
    Serial.println("Configuração de pinos:");
    Serial.println("- LED Conectado: GPIO" + String(LED_CONNECTED_PIN));
    Serial.println("- LED Erro: GPIO" + String(LED_ERROR_PIN));
    Serial.println("- LED Transmitindo: GPIO" + String(LED_TRANSMIT_PIN));
    Serial.println("- Relé Porta: GPIO" + String(DOOR_RELAY_PIN));
    Serial.println("- Botão Local: GPIO" + String(BUTTON_PIN));
}

void DeviceController::testRelay() {
    if (!initialized) {
        Serial.println("ERRO: DeviceController não inicializado");
        return;
    }
    
    Serial.println("=== TESTE DO RELÉ ===");
    Serial.println("Ativando relé por 2 segundos...");
    
    digitalWrite(DOOR_RELAY_PIN, HIGH);
    Serial.println("Relé ATIVADO (HIGH)");
    delay(2000);
    
    digitalWrite(DOOR_RELAY_PIN, LOW);
    Serial.println("Relé DESATIVADO (LOW)");
    
    Serial.println("=== FIM DO TESTE ===");
}

void DeviceController::debounceButton() {
    delay(50); // Debounce básico
}

void DeviceController::handleButtonPress() {
    Serial.println("Botão pressionado!");
    
    // Alternar estado da porta
    if (doorOpen) {
        closeDoor();
        Serial.println("Botão local: FECHAR porta");
    } else {
        openDoor();
        Serial.println("Botão local: ABRIR porta");
    }
    
    // Enviar comando local (será implementado quando necessário)
    String status = doorOpen ? "aberto" : "fechado";
    Serial.println("Status local atualizado: " + status);
}
