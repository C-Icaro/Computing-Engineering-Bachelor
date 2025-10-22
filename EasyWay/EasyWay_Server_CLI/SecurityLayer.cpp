#include "SecurityLayer.h"

SecurityLayer::SecurityLayer() {
    secretKey = "EasyWay2024";
}

SecurityLayer::~SecurityLayer() {
}

void SecurityLayer::initialize() {
    Serial.println("Camada de segurança inicializada");
}

void SecurityLayer::setSecretKey(const String& key) {
    secretKey = key;
}

String SecurityLayer::createMessage(const String& id, const String& type, const String& data) {
    DynamicJsonDocument doc(512);
    
    doc["id"] = id;
    doc["type"] = type;
    doc["data"] = data;
    doc["timestamp"] = generateTimestamp();
    
    // Adicionar comando específico baseado no tipo
    if (type == "command") {
        doc["cmd"] = data;
    } else if (type == "message") {
        doc["message"] = data;
    } else if (type == "status") {
        doc["status"] = data;
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Calcular checksum
    uint32_t checksum = calculateChecksum(jsonString);
    doc["checksum"] = checksum;
    
    String finalMessage;
    serializeJson(doc, finalMessage);
    
    return finalMessage;
}

String SecurityLayer::createAckMessage(const String& id) {
    return createMessage(id, "ack", "received");
}

String SecurityLayer::createCommandMessage(const String& id, const String& command) {
    return createMessage(id, "command", command);
}

bool SecurityLayer::validateMessage(const String& message) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("ERRO: Falha ao parsear JSON");
        return false;
    }
    
    // Verificar campos obrigatórios
    if (!doc.containsKey("id") || !doc.containsKey("type") || !doc.containsKey("checksum")) {
        Serial.println("ERRO: Campos obrigatórios ausentes");
        return false;
    }
    
    // Validar ID
    if (!isValidId(doc["id"])) {
        Serial.println("ERRO: ID inválido");
        return false;
    }
    
    // Validar checksum
    String messageWithoutChecksum = message;
    messageWithoutChecksum.replace("\"checksum\":" + String(doc["checksum"].as<uint32_t>()), "");
    messageWithoutChecksum.replace(",,", ",");
    messageWithoutChecksum.replace("{,", "{");
    messageWithoutChecksum.replace(",}", "}");
    
    uint32_t calculatedChecksum = calculateChecksum(messageWithoutChecksum);
    uint32_t receivedChecksum = doc["checksum"];
    
    if (calculatedChecksum != receivedChecksum) {
        Serial.println("ERRO: Checksum inválido");
        Serial.print("Calculado: ");
        Serial.println(calculatedChecksum);
        Serial.print("Recebido: ");
        Serial.println(receivedChecksum);
        return false;
    }
    
    return true;
}

bool SecurityLayer::validateChecksum(const String& message, uint32_t checksum) {
    uint32_t calculatedChecksum = calculateChecksum(message);
    return calculatedChecksum == checksum;
}

JsonObject SecurityLayer::parseMessage(const String& message) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("ERRO: Falha ao parsear mensagem JSON");
        return JsonObject();
    }
    
    return doc.as<JsonObject>();
}

String SecurityLayer::extractId(const String& message) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        return "";
    }
    
    return doc["id"].as<String>();
}

String SecurityLayer::extractType(const String& message) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        return "";
    }
    
    return doc["type"].as<String>();
}

String SecurityLayer::extractData(const String& message) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        return "";
    }
    
    String type = doc["type"].as<String>();
    
    if (type == "command") {
        return doc["cmd"].as<String>();
    } else if (type == "message") {
        return doc["message"].as<String>();
    } else if (type == "status") {
        return doc["status"].as<String>();
    }
    
    return doc["data"].as<String>();
}

uint32_t SecurityLayer::calculateChecksum(const String& data) {
    crc32.reset();
    crc32.update(data.c_str(), data.length());
    return crc32.finalize();
}

String SecurityLayer::generateTimestamp() {
    return String(millis());
}

bool SecurityLayer::isValidId(const String& id) {
    // Validar formato do ID (ex: cliente01, servidor01, etc.)
    if (id.length() < 3 || id.length() > 20) {
        return false;
    }
    
    // Verificar se contém apenas caracteres alfanuméricos
    for (int i = 0; i < id.length(); i++) {
        char c = id.charAt(i);
        if (!isalnum(c)) {
            return false;
        }
    }
    
    return true;
}
