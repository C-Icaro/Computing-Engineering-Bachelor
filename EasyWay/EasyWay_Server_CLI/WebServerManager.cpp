#include "WebServerManager.h"

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
    
    // Verificar arquivos no SPIFFS
    Serial.println("Verificando arquivos no SPIFFS:");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.print("Arquivo encontrado: ");
        Serial.print(file.name());
        Serial.print(" (");
        Serial.print(file.size());
        Serial.println(" bytes)");
        file.close();
        file = root.openNextFile();
    }
    root.close();
    
    // Configurar modo AP (Access Point)
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    
    // Configurar servidor
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
    // Implementação simplificada - as rotas serão tratadas no handleClient
    Serial.print("Rota adicionada: ");
    Serial.print(method);
    Serial.print(" ");
    Serial.println(path);
}

void WebServerManager::serveStatic(const String& path, const String& file) {
    // Implementação simplificada - arquivos estáticos serão tratados no handleClient
    Serial.print("Arquivo estático: ");
    Serial.print(path);
    Serial.print(" -> ");
    Serial.println(file);
}

void WebServerManager::handleClient() {
    if (!initialized) {
        return;
    }
    
    WiFiClient client = server.available();
    if (client) {
        handleHttpRequest(client);
        client.stop();
    }
}

void WebServerManager::handleHttpRequest(WiFiClient& client) {
    String request = "";
    while (client.connected()) {
        if (client.available()) {
            char c = client.read();
            request += c;
            if (c == '\n') {
                break;
            }
        }
    }
    
    if (request.length() > 0) {
        String method = parseHttpMethod(request);
        String path = parseHttpPath(request);
        
        Serial.print("Requisição: ");
        Serial.print(method);
        Serial.print(" ");
        Serial.println(path);
        
        // Rota raiz
        if (path == "/" || path == "/index.html") {
            handleRoot();
            sendHtmlPage(client);
        }
        // API de status
        else if (path == "/api/status") {
            String response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: application/json\r\n";
            response += "Connection: close\r\n\r\n";
            response += "{\"status\":\"online\",\"uptime\":" + String(millis()) + "}";
            client.print(response);
        }
        // Arquivos estáticos
        else if (path == "/style.css") {
            sendCssFile(client);
        }
        else if (path == "/script.js") {
            sendJsFile(client);
        }
        // Página não encontrada
        else {
            handleNotFound();
            String response = "HTTP/1.1 404 Not Found\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Connection: close\r\n\r\n";
            response += "Página não encontrada";
            client.print(response);
        }
    }
}

String WebServerManager::parseHttpMethod(const String& request) {
    int spaceIndex = request.indexOf(' ');
    if (spaceIndex > 0) {
        return request.substring(0, spaceIndex);
    }
    return "";
}

String WebServerManager::parseHttpPath(const String& request) {
    int firstSpace = request.indexOf(' ');
    int secondSpace = request.indexOf(' ', firstSpace + 1);
    if (firstSpace > 0 && secondSpace > firstSpace) {
        return request.substring(firstSpace + 1, secondSpace);
    }
    return "";
}

void WebServerManager::sendResponse(int code, const String& contentType, const String& content) {
    // Implementação simplificada - será tratada no handleHttpRequest
    Serial.print("Resposta HTTP ");
    Serial.print(code);
    Serial.print(" - ");
    Serial.println(contentType);
}

void WebServerManager::sendFile(const String& path, const String& contentType) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("Arquivo não encontrado: " + path);
        return;
    }
    
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Connection: close\r\n\r\n";
    
    // Enviar cabeçalho
    WiFiClient client = server.available();
    if (client) {
        client.print(response);
        
        // Enviar conteúdo do arquivo
        while (file.available()) {
            client.write(file.read());
        }
        
        client.stop();
    }
    
    file.close();
}

void WebServerManager::sendFileToClient(WiFiClient& client, const String& path, const String& contentType) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("Arquivo não encontrado: " + path);
        String response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Connection: close\r\n\r\n";
        response += "Arquivo não encontrado: " + path;
        client.print(response);
        return;
    }
    
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Connection: close\r\n\r\n";
    
    client.print(response);
    
    // Enviar conteúdo do arquivo
    while (file.available()) {
        client.write(file.read());
    }
    
    file.close();
}

void WebServerManager::sendHtmlPage(WiFiClient& client) {
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Connection: close\r\n\r\n";
    
    String html = "<!DOCTYPE html><html lang='pt-BR'><head>";
    html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>EasyWay - Sistema IoT</title>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#f5f5f5;}";
    html += ".container{max-width:800px;margin:0 auto;background:white;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    html += ".header{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;padding:30px;text-align:center;border-radius:10px 10px 0 0;}";
    html += ".header h1{margin:0;font-size:2.5em;}.header p{margin:10px 0 0 0;opacity:0.9;}";
    html += ".status{background:#4CAF50;color:white;padding:10px;border-radius:5px;margin:20px;text-align:center;}";
    html += ".content{padding:30px;}.section{margin:20px 0;padding:20px;border:1px solid #ddd;border-radius:5px;}";
    html += ".btn{background:#667eea;color:white;border:none;padding:10px 20px;border-radius:5px;cursor:pointer;margin:5px;}";
    html += ".btn:hover{background:#5a6fd8;}.device{background:#f9f9f9;padding:15px;margin:10px 0;border-radius:5px;}";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<div class='header'><h1>🚀 EasyWay Server</h1><p>Sistema de Comunicação IoT Local</p></div>";
    html += "<div class='status'>✅ Servidor Online - Uptime: " + String(millis()/1000) + "s</div>";
    html += "<div class='content'>";
    html += "<div class='section'><h2>📡 Status do Sistema</h2>";
    html += "<p><strong>SSID:</strong> EasyWay-Server</p>";
    html += "<p><strong>IP:</strong> 192.168.4.1</p>";
    html += "<p><strong>Porta:</strong> 80</p>";
    html += "<p><strong>Status:</strong> <span style='color:green'>Online</span></p></div>";
    html += "<div class='section'><h2>🎮 Controle de Dispositivos</h2>";
    html += "<p>Use os botões abaixo para controlar dispositivos conectados:</p>";
    html += "<button class='btn' onclick='sendCommand(\"abrir\")'>🔓 Abrir Porta</button>";
    html += "<button class='btn' onclick='sendCommand(\"fechar\")'>🔒 Fechar Porta</button>";
    html += "<button class='btn' onclick='sendCommand(\"status\")'>ℹ️ Status</button></div>";
    html += "<div class='section'><h2>📊 Dispositivos Conectados</h2>";
    html += "<div id='devices'>Carregando dispositivos...</div></div>";
    html += "<div class='section'><h2>📝 Logs do Sistema</h2>";
    html += "<div id='logs' style='background:#f0f0f0;padding:10px;border-radius:5px;max-height:200px;overflow-y:auto;'>";
    html += "Sistema inicializado com sucesso!<br>";
    html += "Servidor web ativo na porta 80<br>";
    html += "Aguardando conexões...<br></div></div></div></div>";
    html += "<script>";
    html += "function sendCommand(cmd){alert('Comando: '+cmd+'\\n\\nFuncionalidade será implementada em breve!');}";
    html += "setInterval(function(){document.getElementById('logs').innerHTML+='['+new Date().toLocaleTimeString()+'] Sistema ativo<br>';},5000);";
    html += "</script></body></html>";
    
    client.print(response);
    client.print(html);
}

void WebServerManager::sendCssFile(WiFiClient& client) {
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/css\r\n";
    response += "Connection: close\r\n\r\n";
    
    String css = "/* EasyWay Server CSS */";
    css += "body{font-family:Arial,sans-serif;margin:0;padding:0;background:#f5f5f5;}";
    css += ".container{max-width:800px;margin:0 auto;background:white;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    css += ".header{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;padding:30px;text-align:center;border-radius:10px 10px 0 0;}";
    css += ".btn{background:#667eea;color:white;border:none;padding:10px 20px;border-radius:5px;cursor:pointer;margin:5px;}";
    css += ".btn:hover{background:#5a6fd8;}";
    
    client.print(response);
    client.print(css);
}

void WebServerManager::sendJsFile(WiFiClient& client) {
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: application/javascript\r\n";
    response += "Connection: close\r\n\r\n";
    
    String js = "// EasyWay Server JavaScript";
    js += "console.log('EasyWay Server JS carregado');";
    js += "function sendCommand(cmd){console.log('Comando:',cmd);}";
    js += "function refreshDevices(){console.log('Atualizando dispositivos...');}";
    
    client.print(response);
    client.print(js);
}

String WebServerManager::getPathParameter(const String& name) {
    // Implementação simplificada
    return "";
}

String WebServerManager::getBodyParameter(const String& name) {
    // Implementação simplificada
    return "";
}

String WebServerManager::getQueryParameter(const String& name) {
    // Implementação simplificada
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
    // Rotas padrão serão tratadas no handleHttpRequest
    Serial.println("Rotas padrão configuradas");
}

void WebServerManager::handleRoot() {
    Serial.println("Requisição para página raiz");
}

void WebServerManager::handleNotFound() {
    Serial.println("Página não encontrada");
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