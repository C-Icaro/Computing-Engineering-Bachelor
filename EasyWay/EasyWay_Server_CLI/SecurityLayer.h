#ifndef SECURITY_LAYER_H
#define SECURITY_LAYER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <CRC32.h>

class SecurityLayer {
private:
    CRC32 crc32;
    String secretKey;
    
public:
    SecurityLayer();
    ~SecurityLayer();
    
    // Inicialização
    void initialize();
    void setSecretKey(const String& key);
    
    // Criação de mensagens
    String createMessage(const String& id, const String& type, const String& data);
    String createAckMessage(const String& id);
    String createCommandMessage(const String& id, const String& command);
    
    // Validação
    bool validateMessage(const String& message);
    bool validateChecksum(const String& message, uint32_t checksum);
    
    // Parsing
    JsonObject parseMessage(const String& message);
    String extractId(const String& message);
    String extractType(const String& message);
    String extractData(const String& message);
    
    // Utilitários
    uint32_t calculateChecksum(const String& data);
    String generateTimestamp();
    bool isValidId(const String& id);
};

#endif
