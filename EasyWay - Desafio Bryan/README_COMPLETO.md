# EasyWay - Sistema de Comunicação IoT Local

## 📋 Visão Geral

O EasyWay é um sistema completo de comunicação IoT local que permite controle remoto de dispositivos e troca de mensagens através de comunicação via Wi-Fi direta, sem dependência de redes externas.

## 🏗️ Estrutura do Projeto

```
EasyWay/
├── Server/                    # Código do dispositivo servidor
│   ├── EasyWay_Server.ino    # Arquivo principal do servidor
│   ├── WiFiCommunication.h   # Comunicação via Wi-Fi
│   ├── WiFiCommunication.cpp
│   ├── SecurityLayer.h        # Camada de segurança
│   ├── SecurityLayer.cpp
│   ├── WebServer.h           # Servidor web
│   ├── WebServer.cpp
│   ├── DeviceManager.h       # Gerenciamento de dispositivos
│   ├── DeviceManager.cpp
│   └── data/                 # Arquivos da interface web
│       ├── index.html
│       ├── style.css
│       └── script.js
└── Client/                   # Código do dispositivo cliente
    ├── EasyWay_Client.ino    # Arquivo principal do cliente
    ├── WiFiCommunication.h   # Comunicação via Wi-Fi
    ├── WiFiCommunication.cpp
    ├── SecurityLayer.h        # Camada de segurança
    ├── SecurityLayer.cpp
    ├── DeviceController.h     # Controle de dispositivos
    ├── DeviceController.cpp
    ├── LEDManager.h          # Gerenciamento de LEDs
    └── LEDManager.cpp
```

## 🔧 Componentes Necessários

### Para o Servidor:
- ESP32-WROOM-32U (com antena externa IPEX)
- LED de status
- Resistor 220Ω para LED

### Para o Cliente:
- ESP32-WROOM-32U
- 3 LEDs (Verde, Vermelho, Amarelo)
- Resistor 220Ω para cada LED
- Relé para controle da porta
- Botão para comandos locais
- Resistor 10kΩ para pull-up do botão

## 📚 Bibliotecas Necessárias

Instale as seguintes bibliotecas no Arduino IDE:

```cpp
// Para comunicação via Wi-Fi
#include <WiFi.h>
#include <WiFiUdp.h>

// Para servidor web
#include <WebServer.h>
#include <SPIFFS.h>

// Para JSON e segurança
#include <ArduinoJson.h>
#include <CRC32.h>
```

## ⚙️ Configuração

### 1. Configuração do Servidor

1. Carregue o código do servidor (`EasyWay_Server.ino`) na ESP32
2. Faça upload dos arquivos da pasta `data/` para o SPIFFS da ESP32
3. Conecte o módulo RF24:
   - CE → Pino 4
   - CSN → Pino 5
   - MOSI → Pino 23
   - MISO → Pino 19
   - SCK → Pino 18
   - VCC → 3.3V
   - GND → GND

### 2. Configuração do Cliente

1. Carregue o código do cliente (`EasyWay_Client.ino`) na ESP32
2. Conecte os componentes:
   - RF24 (mesma conexão do servidor)
   - LED Verde → Pino 2
   - LED Vermelho → Pino 3
   - LED Amarelo → Pino 4
   - Relé → Pino 6
   - Botão → Pino 7

### 3. Configuração da Rede

O servidor cria uma rede WiFi local:
- **SSID**: EasyWay-Server
- **Senha**: easyway123
- **IP**: 192.168.4.1

## 🚀 Como Usar

### 1. Inicialização

1. Ligue o dispositivo servidor primeiro
2. Aguarde a criação da rede WiFi "EasyWay-Server"
3. Ligue os dispositivos cliente
4. Conecte-se à rede WiFi do servidor

### 2. Acesso à Interface Web

1. Abra um navegador web
2. Acesse: `http://192.168.4.1`
3. A interface web será carregada automaticamente

### 3. Funcionalidades Disponíveis

#### Aba Dispositivos:
- Visualizar todos os dispositivos conectados
- Status em tempo real (online/offline)
- Informações de cada dispositivo

#### Aba Controle:
- Selecionar dispositivo
- Enviar comandos (abrir/fechar porta)
- Enviar mensagens de texto

#### Aba Mensagens:
- Visualizar histórico de mensagens
- Mensagens enviadas e recebidas

#### Aba Logs:
- Logs detalhados do sistema
- Histórico de comandos e atividades

## 🔒 Segurança

O sistema implementa várias camadas de segurança:

1. **ID Único**: Cada dispositivo possui um identificador único
2. **Checksum CRC32**: Validação de integridade dos pacotes
3. **Protocolo JSON**: Formato padronizado de comunicação
4. **Confirmação ACK/NACK**: Confirmação de recebimento
5. **Validação de Timestamp**: Controle de tempo das mensagens

## 📡 Protocolo de Comunicação

### Formato das Mensagens

```json
{
  "id": "cliente01",
  "type": "command",
  "cmd": "abrir",
  "timestamp": "1234567890",
  "checksum": 1234567890
}
```

### Tipos de Mensagem

- **command**: Comandos de controle
- **status**: Atualização de status
- **message**: Mensagens de texto
- **ack**: Confirmação de recebimento
- **heartbeat**: Sinal de presença

## 🐛 Solução de Problemas

### Problemas Comuns:

1. **Dispositivo não conecta**:
   - Verifique as conexões do RF24
   - Confirme se o servidor está ligado primeiro
   - Verifique a alimentação (3.3V)

2. **Interface web não carrega**:
   - Verifique se os arquivos foram enviados para o SPIFFS
   - Confirme a conexão WiFi
   - Teste o IP: 192.168.4.1

3. **Comandos não funcionam**:
   - Verifique se o dispositivo está online
   - Confirme a comunicação via rádio
   - Verifique os logs do sistema

### Logs de Debug:

Monitore o Serial Monitor para mensagens de debug:
- Baud rate: 115200
- Mensagens detalhadas de cada operação

## 🔧 Personalização

### Alterar IDs dos Dispositivos:

No arquivo `EasyWay_Client.ino`, modifique:
```cpp
const String DEVICE_ID = "cliente01"; // Altere para o ID desejado
```

### Alterar Configurações de Rede:

No arquivo `EasyWay_Server.ino`, modifique:
```cpp
const char* WIFI_SSID = "EasyWay-Server";
const char* WIFI_PASSWORD = "easyway123";
```

### Adicionar Novos Comandos:

1. Modifique o `DeviceController.cpp` para novos comandos
2. Atualize a interface web em `script.js`
3. Adicione os novos tipos de mensagem em `SecurityLayer.cpp`

## 📈 Expansões Futuras

- Suporte a mais dispositivos
- Interface mobile (PWA)
- Notificações push
- Histórico persistente
- Backup automático
- Modo sleep avançado

## 👥 Desenvolvimento

Este projeto foi desenvolvido como parte do Desafio Bryan Kano no Instituto de Tecnologia e Inovação (Inteli), módulo 04, turma 18.

### Características Técnicas:
- **Arquitetura**: Cliente/Servidor
- **Comunicação**: Half-duplex via rádio
- **Interface**: Web local responsiva
- **Segurança**: Múltiplas camadas
- **Escalabilidade**: Modular e expansível

## 📄 Licença

Este projeto é desenvolvido para fins educacionais no contexto do Inteli.
