# EasyWay Client CLI - Configuração

## 📋 Estrutura do Projeto

```
EasyWay_Client_CLI/
├── EasyWay_Client_CLI.ino    # Arquivo principal
├── WiFiCommunication.h        # Comunicação Wi-Fi UDP
├── WiFiCommunication.cpp
├── DeviceController.h        # Controle de dispositivos físicos
├── DeviceController.cpp
├── SecurityLayer.h           # Camada de segurança
├── SecurityLayer.cpp
├── StatusManager.h           # Gerenciamento de status
├── StatusManager.cpp
└── README.md                 # Documentação
```

## 🔧 Configurações

### Pinos do ESP32 (Seguros)

| Função | GPIO | Descrição |
|--------|------|-----------|
| LED Conectado | 2 | Verde - Indica conexão ativa |
| LED Erro | 4 | Vermelho - Indica erro |
| LED Transmitindo | 5 | Amarelo - Indica transmissão |
| Relé Porta | 13 | Controle da porta/relé |
| Botão Local | 14 | Comandos locais |

### Configurações de Rede

```cpp
const char* WIFI_SSID = "EasyWay-Server";
const char* WIFI_PASSWORD = "easyway123";
const int SERVER_PORT = 8080;
const String DEVICE_ID = "cliente01";
```

## 🚀 Como Usar

### 1. Compilação

```bash
arduino-cli compile --fqbn esp32:esp32:esp32da .
```

### 2. Upload

```bash
arduino-cli upload --fqbn esp32:esp32:esp32da --port COM3 .
```

### 3. Monitoramento

```bash
arduino-cli monitor --port COM3
```

## 📡 Protocolo de Comunicação

### Mensagens Enviadas

- **Presença**: `{"id":"cliente01","type":"status","status":"online"}`
- **Heartbeat**: `{"id":"cliente01","type":"heartbeat","status":"ping"}`
- **Status**: `{"id":"cliente01","type":"status","status":"aberto/fechado"}`

### Mensagens Recebidas

- **Comando**: `{"id":"cliente01","type":"command","cmd":"abrir/fechar/status"}`
- **ACK**: `{"id":"cliente01","type":"ack","status":"received"}`

## 🔍 Debug e Logs

### Serial Monitor Output

```
EasyWay Client CLI - Iniciando sistema...
Camada de segurança inicializada
Controlador de dispositivos inicializado
Gerenciador de status inicializado
Comunicação WiFi inicializada com sucesso
SSID: EasyWay-Server
IP do dispositivo: 192.168.4.2
IP do servidor: 192.168.4.1
Porta: 8080
EasyWay Client CLI - Sistema inicializado com sucesso!
ID do dispositivo: cliente01
Conectado à rede: EasyWay-Server
Sinal de presença enviado
```

### LEDs de Status

- **Verde (GPIO2)**: Conectado ao servidor
- **Vermelho (GPIO4)**: Erro de conexão
- **Amarelo (GPIO5)**: Transmitindo dados (200ms)

## 🎮 Controles

### Botão Local (GPIO14)

- **Pressionar**: Alterna estado da porta
- **Debounce**: 50ms
- **Ação**: Abre/fecha porta + envia status

### Comandos Remotos

- **abrir**: Abre porta por 1 segundo
- **fechar**: Fecha porta
- **status**: Retorna status atual

## 🔒 Segurança

- **Checksum CRC32**: Validação de integridade
- **IDs Válidos**: Formato alfanumérico
- **Timestamps**: Controle temporal
- **Campos Obrigatórios**: Estrutura JSON validada

## 📊 Status do Sistema

### Indicadores

- **Conectado**: LED verde + logs
- **Erro**: LED vermelho + mensagem de erro
- **Transmitindo**: LED amarelo temporário
- **Heartbeat**: A cada 30 segundos

### Monitoramento

- **Serial Monitor**: Logs detalhados
- **LEDs**: Status visual
- **Rede**: Informações de conexão

## 🛠️ Solução de Problemas

### Problemas Comuns

1. **Não conecta**: Verificar SSID/senha
2. **LED vermelho**: Erro de conexão
3. **Sem resposta**: Verificar servidor ativo
4. **Comandos não funcionam**: Verificar pinos

### Debug

- Ativar Serial Monitor (115200 baud)
- Verificar logs de inicialização
- Testar botão local
- Monitorar LEDs de status

## 📈 Próximas Versões

- [ ] Interface web local
- [ ] Configuração OTA
- [ ] Múltiplos dispositivos
- [ ] Logs persistentes
- [ ] Configuração dinâmica

---

**Desenvolvido para Inteli - Instituto de Tecnologia e Liderança**
