#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

class WebServerManager {
private:
    WebServer server;
    bool initialized;
    String ssid;
    String password;
    
public:
    WebServerManager();
    ~WebServerManager();
    
    // Inicialização
    bool initialize(const char* ssid, const char* password);
    void shutdown();
    
    // Gerenciamento de rotas
    void addRoute(const String& path, const String& method, void (*handler)());
    void serveStatic(const String& path, const String& file);
    
    // Handlers HTTP
    void handleClient();
    void sendResponse(int code, const String& contentType, const String& content);
    void sendFile(const String& path, const String& contentType);
    
    // Parâmetros de requisição
    String getPathParameter(const String& name);
    String getBodyParameter(const String& name);
    String getQueryParameter(const String& name);
    
    // Status
    bool isInitialized() const;
    IPAddress getServerIP() const;
    void printNetworkInfo();
    
private:
    void setupDefaultRoutes();
    void handleRoot();
    void handleNotFound();
    String getContentType(const String& filename);
};

#endif
