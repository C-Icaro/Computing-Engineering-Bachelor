# Estrutura Modular do Firmware ESP32-CAM

Este firmware foi modularizado para facilitar a manutenção e compreensão do código.

## Estrutura de Arquivos

```
firmware/
├── firmware.ino          # Arquivo principal (setup e loop)
├── config.h              # Todas as configurações (WiFi, MQTT, Camera, YOLO)
├── utils.h               # Funções auxiliares (codificação base64)
├── WiFiConnector.h       # Classe para gerenciar conexão WiFi
├── CameraController.h    # Classe para controlar a câmera ESP32-CAM
├── YoloController.h      # Classe para gerenciar detecção YOLO
├── MQTTPublisher.h       # Classe para publicar frames via MQTT
└── http_server.h         # Servidor HTTP e handlers (stream, API, interface web)
```

## Descrição dos Módulos

### `firmware.ino`
Arquivo principal que contém:
- Inicialização do sistema (`setup()`)
- Loop principal (`loop()`)
- Instâncias globais dos controladores

### `config.h`
Centraliza todas as configurações:
- **Câmera**: Definições de pinos para ESP32-CAM AI-Thinker
- **WiFi**: SSID e senha da rede
- **MQTT**: Broker, credenciais, tópicos e configurações de publicação
- **YOLO**: Endpoint de inferência (opcional)

### `utils.h`
Funções utilitárias:
- `base64EncodeChunk()`: Codificação base64 otimizada para economizar RAM

### `WiFiConnector.h`
Classe responsável por:
- Conectar à rede WiFi
- Gerenciar timeout de conexão
- Exibir status da conexão

### `CameraController.h`
Classe responsável por:
- Inicializar a câmera ESP32-CAM
- Configurar parâmetros do sensor (qualidade, brilho, contraste, etc.)
- Detectar PSRAM e ajustar configurações automaticamente
- Fornecer acesso ao sensor para ajustes

### `YoloController.h`
Classe responsável por:
- Gerenciar estado de detecção YOLO (ativado/desativado)
- Processar frames para inferência
- Configurar endpoint de inferência externa

### `MQTTPublisher.h`
Classe responsável por:
- Conectar ao broker MQTT (HiveMQ Cloud)
- Publicar frames JPEG codificados em base64 via MQTT
- Publicar status do dispositivo
- Processar comandos remotos (toggle YOLO, toggle MQTT, restart)
- Gerenciar reconexão automática

### `http_server.h`
Módulo do servidor HTTP que contém:
- Interface web HTML completa
- Handler para stream MJPEG (`/stream`)
- Handler para status da câmera (`/status`)
- API REST para YOLO (`/api/yolo`, `/api/yolo/toggle`)
- Função `startCameraServer()` para inicializar o servidor

## Fluxo de Dados

1. **Setup**: Inicializa câmera → Conecta WiFi → Inicializa YOLO → Inicializa MQTT → Inicia servidor HTTP
2. **Loop**: Processa mensagens MQTT continuamente
3. **Stream HTTP**: Captura frame → Processa YOLO → Envia via HTTP → Publica via MQTT

## Dependências

- `esp_camera.h` - Driver da câmera ESP32
- `esp_http_server.h` - Servidor HTTP do ESP-IDF
- `WiFi.h` - WiFi do ESP32
- `PubSubClient.h` - Cliente MQTT
- `ArduinoJson.h` - Parsing JSON
- `WiFiClientSecure.h` - Cliente WiFi seguro (TLS)

## Como Modificar

### Alterar Configurações
Edite `config.h` para modificar:
- Credenciais WiFi
- Configurações MQTT
- Parâmetros da câmera

### Adicionar Nova Funcionalidade
1. Crie um novo arquivo `.h` para a funcionalidade
2. Inclua no `firmware.ino`
3. Instancie no `setup()` se necessário

### Modificar Interface Web
Edite o HTML em `http_server.h` (variável `INDEX_HTML`)

## Vantagens da Modularização

✅ **Manutenibilidade**: Cada módulo tem responsabilidade única  
✅ **Legibilidade**: Código mais fácil de entender  
✅ **Reutilização**: Módulos podem ser reutilizados em outros projetos  
✅ **Testabilidade**: Módulos podem ser testados isoladamente  
✅ **Colaboração**: Múltiplos desenvolvedores podem trabalhar em módulos diferentes

