# EasyWay - Arquitetura do Sistema (Versão Wi-Fi)

## 🏗️ Diagrama de Arquitetura

```
┌─────────────────────────────────────────────────────────────────┐
│                        EASYWAY SYSTEM                          │
│                    Comunicação IoT Local                       │
│                      (Wi-Fi Direto)                           │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   ESP32 SERVER  │    │  ESP32 CLIENT   │    │  ESP32 CLIENT   │
│                 │    │                 │    │                 │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │ Web Server  │ │    │ │Device Ctrl  │ │    │ │Device Ctrl  │ │
│ │ (HTTP API)  │ │    │ │             │ │    │ │             │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │Device Mgr   │ │    │ │ LED Manager │ │    │ │ LED Manager │ │
│ │             │ │    │ │             │ │    │ │             │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │Security     │ │    │ │Security     │ │    │ │Security     │ │
│ │Layer        │ │    │ │Layer        │ │    │ │Layer        │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │WiFi Comm    │ │◄──►│ │WiFi Comm    │ │◄──►│ │WiFi Comm    │ │
│ │(UDP Direct) │ │    │ │(UDP Direct) │ │    │ │(UDP Direct) │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   WiFi AP       │    │   Hardware      │    │   Hardware      │
│ EasyWay-Server  │    │   Controls      │    │   Controls      │
│ 192.168.4.1     │    │                 │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Browser   │    │   Door Relay    │    │   Door Relay    │
│   Interface     │    │   LEDs (3x)      │    │   LEDs (3x)      │
│                 │    │   Button        │    │   Button        │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 🔄 Fluxo de Comunicação Wi-Fi

### 1. Inicialização:
```
Servidor: Cria WiFi AP "EasyWay-Server"
Cliente:  Conecta ao WiFi AP
Cliente:  Envia sinal de presença via UDP
Servidor: Registra dispositivo
Cliente:  LED Verde liga (conectado)
```

### 2. Comando via Web:
```
Browser → Servidor: POST /api/devices/{id}/control
Servidor → Cliente: Comando via UDP WiFi
Cliente → Servidor: Confirmação (ACK)
Servidor → Browser: Resposta JSON
```

### 3. Heartbeat:
```
Cliente → Servidor: Heartbeat via UDP (30s)
Servidor → Cliente: ACK via UDP
Cliente: LED Verde mantém (online)
```

### 4. Comando Local:
```
Botão → Cliente: Comando Local
Cliente: Executa ação
Cliente → Servidor: Status Update via UDP
```

## 📡 Protocolo de Comunicação Wi-Fi

### Formato UDP Padrão:
```
Pacote UDP: "deviceId|JSON_MESSAGE"
Exemplo: "cliente01|{\"id\":\"cliente01\",\"type\":\"command\",\"cmd\":\"abrir\"}"
```

### Formato JSON Padrão:
```json
{
  "id": "cliente01",
  "type": "command|status|message|ack|heartbeat",
  "cmd": "abrir|fechar|status",
  "status": "online|offline|aberto|fechado",
  "message": "texto da mensagem",
  "timestamp": "1234567890",
  "checksum": 1234567890
}
```

### Tipos de Mensagem:

#### Command (Comando):
```json
{
  "id": "cliente01",
  "type": "command",
  "cmd": "abrir",
  "timestamp": "1234567890",
  "checksum": 1234567890
}
```

#### Status (Status):
```json
{
  "id": "cliente01",
  "type": "status",
  "status": "aberto",
  "timestamp": "1234567890",
  "checksum": 1234567890
}
```

#### Message (Mensagem):
```json
{
  "id": "cliente01",
  "type": "message",
  "message": "Alerta: Porta aberta há muito tempo",
  "timestamp": "1234567890",
  "checksum": 1234567890
}
```

#### ACK (Confirmação):
```json
{
  "id": "cliente01",
  "type": "ack",
  "data": "received",
  "timestamp": "1234567890",
  "checksum": 1234567890
}
```

## 🔒 Camada de Segurança

### Validações Implementadas:

1. **ID Único**: Cada dispositivo tem identificador único
2. **Checksum CRC32**: Validação de integridade
3. **Timestamp**: Controle de tempo das mensagens
4. **ACK/NACK**: Confirmação de recebimento
5. **Formato JSON**: Estrutura padronizada
6. **Rede Privada**: WiFi AP isolado

### Processo de Validação:
```
Mensagem UDP Recebida
       ↓
Extrair Device ID
       ↓
Validar JSON
       ↓
Verificar Campos Obrigatórios
       ↓
Validar ID
       ↓
Calcular Checksum
       ↓
Comparar Checksums
       ↓
Processar Mensagem
       ↓
Enviar ACK via UDP
```

## 🌐 Interface Web

### Estrutura da Interface:
```
┌─────────────────────────────────────────┐
│                HEADER                   │
│  🛰️ EasyWay - Sistema IoT Local        │
│  Status: ● Servidor Online             │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│                NAVIGATION               │
│ [Dispositivos] [Controle] [Mensagens] [Logs] │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│                CONTENT                  │
│                                         │
│  ┌─────────────┐ ┌─────────────┐       │
│  │   Stats     │ │   Stats     │       │
│  │ Online: 2   │ │ Total: 3    │       │
│  └─────────────┘ └─────────────┘       │
│                                         │
│  ┌─────────────────────────────────────┐ │
│  │        Device Cards                 │ │
│  │  cliente01 ● Online                 │ │
│  │  Status: Aberto | Último: Abrir     │ │
│  │  [Abrir] [Fechar] [Status]          │ │
│  └─────────────────────────────────────┘ │
└─────────────────────────────────────────┘
```

### APIs Disponíveis:

#### GET /api/devices
Retorna lista de dispositivos conectados

#### POST /api/devices/{id}/control
Envia comando para dispositivo específico

#### POST /api/message
Envia mensagem para dispositivo

#### GET /api/logs
Retorna logs do sistema

#### GET /api/status
Retorna status do servidor

## 🔧 Hardware

### Servidor (ESP32):
```
┌─────────────────┐
│     ESP32       │
│  ┌───────────┐  │
│  │   GPIO2   │──┼──► LED Status
│  │   GPIO4   │──┼──► Antena Externa (IPEX)
│  │   3.3V    │──┼──► Alimentação
│  │   GND     │──┼──► Terra
│  └───────────┘  │
└─────────────────┘
```

### Cliente (ESP32):
```
┌─────────────────┐
│     ESP32       │
│  ┌───────────┐  │
│  │   GPIO2   │──┼──► LED Verde
│  │   GPIO3   │──┼──► LED Vermelho
│  │   GPIO4   │──┼──► LED Amarelo
│  │   GPIO6   │──┼──► Relé Porta
│  │   GPIO7   │──┼──► Botão Local
│  │   GPIO4   │──┼──► Antena Externa (IPEX)
│  │   3.3V    │──┼──► Alimentação
│  │   GND     │──┼──► Terra
│  └───────────┘  │
└─────────────────┘
```

## 📊 Estados dos LEDs

### Cliente - LEDs de Status:

#### LED Verde (Conectado):
- **Ligado**: Conectado ao WiFi e servidor
- **Piscando**: Tentando conectar
- **Apagado**: Desconectado

#### LED Vermelho (Erro):
- **Ligado**: Erro de comunicação WiFi
- **Piscando**: Timeout detectado
- **Apagado**: Sem erros

#### LED Amarelo (Transmitindo):
- **Piscando**: Transmitindo/recebendo dados UDP
- **Apagado**: Sem atividade de rede

### Servidor - LED de Status:
- **Ligado**: Sistema funcionando
- **Apagado**: Sistema com problema

## 🔄 Ciclo de Vida do Sistema

### 1. Boot:
```
Servidor: Inicializa WiFi AP
Servidor: Inicia servidor UDP (porta 8080)
Cliente:  Conecta ao WiFi AP
Cliente:  Envia sinal de presença via UDP
Servidor: Registra dispositivo
Cliente:  LED verde liga
```

### 2. Operação Normal:
```
Cliente:  Heartbeat via UDP (30s)
Servidor: Responde ACK via UDP
Cliente:  Mantém LED verde
```

### 3. Comando:
```
Web:      Envia comando HTTP
Servidor: Valida e envia via UDP
Cliente:  Executa ação
Cliente:  Envia confirmação UDP
Servidor: Atualiza logs
Web:      Mostra resultado
```

### 4. Timeout:
```
Cliente:  Sem resposta UDP (5min)
Cliente:  LED vermelho liga
Cliente:  Entra modo sleep
```

## 🚀 Vantagens da Arquitetura Wi-Fi

### Comparação RF24 vs Wi-Fi:

| Aspecto | RF24 | Wi-Fi (Esta Versão) |
|---------|------|---------------------|
| **Alcance** | ~100m | ~50-100m |
| **Consumo** | Baixo | Médio |
| **Velocidade** | 250kbps | 54Mbps+ |
| **Configuração** | Simples | Simples |
| **Custo** | Módulo adicional | Apenas ESP32 |
| **Interferência** | Baixa | Média |
| **Segurança** | Boa | Excelente |
| **Escalabilidade** | Limitada | Alta |

### Benefícios Específicos:

1. **Sem Hardware Adicional**: Apenas ESP32 necessária
2. **Maior Velocidade**: Comunicação mais rápida
3. **Melhor Debug**: Logs detalhados via Serial
4. **Facilidade de Setup**: Sem conexões físicas extras
5. **Antena Externa**: Melhor alcance com antena IPEX
6. **Protocolo UDP**: Comunicação eficiente e confiável

## 🔧 Configurações de Rede

### Servidor WiFi AP:
```
SSID: EasyWay-Server
Senha: easyway123
IP: 192.168.4.1
Porta UDP: 8080
```

### Cliente WiFi STA:
```
Conecta automaticamente ao AP
IP: 192.168.4.x (DHCP)
Porta UDP: Aleatória (cliente)
```

## 📈 Escalabilidade

### Limitações Atuais:
- **Dispositivos**: Máximo 10 clientes
- **Logs**: Máximo 100 entradas
- **Alcance**: ~50-100m (com antena externa)
- **Concurrent Users**: ~20 (WiFi AP)

### Expansões Futuras:
- **Mais Dispositivos**: Aumentar arrays
- **Maior Alcance**: Antenas de alta potência
- **Mais Usuários**: WiFi mesh
- **Persistência**: SD card
- **Backup**: Cloud sync

## 🔐 Segurança Wi-Fi

### Medidas Implementadas:
- **Rede Privada**: WiFi AP isolado
- **Senha WPA2**: Proteção de acesso
- **UDP Direto**: Sem roteamento externo
- **Checksum CRC32**: Validação de dados
- **IDs Únicos**: Identificação de dispositivos

### Boas Práticas:
- Alterar senha padrão
- Usar IDs não óbvios
- Monitorar logs regularmente
- Manter firmware atualizado
