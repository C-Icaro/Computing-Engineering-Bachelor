#include "WebServer.h"

WebServerManager::WebServerManager() : server(80), initialized(false) {
}

WebServerManager::~WebServerManager() {
    shutdown();
}

bool WebServerManager::initialize(const char* ssid, const char* password) {
    this->ssid = String(ssid);
    this->password = String(password);
    
    // Inicializar SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("ERRO: Falha ao inicializar SPIFFS");
        return false;
    }
    
    // Configurar modo AP (Access Point)
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    
    // Configurar servidor
    setupDefaultRoutes();
    server.begin();
    
    initialized = true;
    Serial.println("Servidor web inicializado com sucesso");
    printNetworkInfo();
    
    return true;
}

void WebServerManager::shutdown() {
    if (initialized) {
        server.stop();
        WiFi.softAPdisconnect(true);
        initialized = false;
        Serial.println("Servidor web desligado");
    }
}

void WebServerManager::addRoute(const String& path, const String& method, void (*handler)()) {
    if (method == "GET") {
        server.on(path, HTTP_GET, handler);
    } else if (method == "POST") {
        server.on(path, HTTP_POST, handler);
    } else if (method == "PUT") {
        server.on(path, HTTP_PUT, handler);
    } else if (method == "DELETE") {
        server.on(path, HTTP_DELETE, handler);
    }
}

void WebServerManager::serveStatic(const String& path, const String& file) {
    server.on(path, [this, file]() {
        sendFile(file, getContentType(file));
    });
}

void WebServerManager::handleClient() {
    if (initialized) {
        server.handleClient();
    }
}

void WebServerManager::sendResponse(int code, const String& contentType, const String& content) {
    server.send(code, contentType, content);
}

void WebServerManager::sendFile(const String& path, const String& contentType) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        server.send(404, "text/plain", "Arquivo não encontrado");
        return;
    }
    
    server.streamFile(file, contentType);
    file.close();
}

String WebServerManager::getPathParameter(const String& name) {
    return server.pathArg(0); // Simplificado - assumindo primeiro parâmetro
}

String WebServerManager::getBodyParameter(const String& name) {
    if (server.hasArg(name)) {
        return server.arg(name);
    }
    return "";
}

String WebServerManager::getQueryParameter(const String& name) {
    if (server.hasArg(name)) {
        return server.arg(name);
    }
    return "";
}

bool WebServerManager::isInitialized() const {
    return initialized;
}

IPAddress WebServerManager::getServerIP() const {
    return WiFi.softAPIP();
}

void WebServerManager::printNetworkInfo() {
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("IP do servidor: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("Endereço MAC: ");
    Serial.println(WiFi.softAPmacAddress());
}

void WebServerManager::setupDefaultRoutes() {
    // Rota raiz
    server.on("/", handleRoot);
    
    // Rota para arquivos não encontrados
    server.onNotFound(handleNotFound);
    
    // Rotas de API básicas
    server.on("/api/status", HTTP_GET, []() {
        server.send(200, "application/json", "{\"status\":\"online\",\"uptime\":" + String(millis()) + "}");
    });
}

void WebServerManager::handleRoot() {
    sendFile("/index.html", "text/html");
}

void WebServerManager::handleNotFound() {
    server.send(404, "text/plain", "Página não encontrada");
}

String WebServerManager::getContentType(const String& filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".json")) return "application/json";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".jpg")) return "image/jpeg";
    else if (filename.endsWith(".gif")) return "image/gif";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else return "text/plain";
}
