#ifndef DEVICE_CONTROLLER_H
#define DEVICE_CONTROLLER_H

#include <Arduino.h>

class DeviceController {
private:
    // Pinos SEGUROS para ESP32 (evitando GPIO6-11 que são SPI Flash)
    const int LED_CONNECTED_PIN = 2;    // Verde - Conectado
    const int LED_ERROR_PIN = 4;        // Vermelho - Erro
    const int LED_TRANSMIT_PIN = 5;     // Amarelo - Transmitindo
    const int DOOR_RELAY_PIN = 13;      // Relé para controle da porta
    const int BUTTON_PIN = 14;          // Botão para comandos locais
    
    bool initialized;
    bool doorOpen;
    unsigned long lastTransmission;
    const unsigned long TRANSMISSION_LED_TIME = 200; // 200ms para LED de transmissão
    
public:
    DeviceController();
    ~DeviceController();
    
    // Inicialização
    void initialize();
    void shutdown();
    
    // Controle da porta
    void openDoor();
    void closeDoor();
    String getDoorStatus() const;
    
    // Comandos locais
    void checkLocalCommands();
    
    // LEDs
    void setConnectedLED(bool state);
    void setErrorLED(bool state);
    void setTransmitLED(bool state);
    void updateLEDs();
    
    // Status
    bool isInitialized() const;
    void printPinConfiguration();
    void testRelay(); // Função para testar o relé
    
private:
    void debounceButton();
    void handleButtonPress();
    void sendLocalCommand(String command);
};

#endif
