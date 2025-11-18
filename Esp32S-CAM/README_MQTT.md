# ESP32-CAM com MQTT + HiveMQ Cloud

Sistema completo para visualização remota de imagens da ESP32-CAM via MQTT usando HiveMQ Cloud.

## 📋 Índice

- [Visão Geral](#visão-geral)
- [Arquitetura](#arquitetura)
- [Configuração do HiveMQ Cloud](#configuração-do-hivemq-cloud)
- [Configuração da ESP32-CAM](#configuração-da-esp32-cam)
- [Instalação de Dependências](#instalação-de-dependências)
- [Uso](#uso)
- [Troubleshooting](#troubleshooting)

## 🎯 Visão Geral

Este projeto permite visualizar imagens da ESP32-CAM de qualquer lugar do mundo através de:

1. **ESP32-CAM**: Captura frames e publica via MQTT
2. **HiveMQ Cloud**: Broker MQTT na nuvem (gratuito)
3. **Cliente Python**: Recebe frames e exibe localmente ou via web

### Funcionalidades

- ✅ Streaming de vídeo via MQTT
- ✅ Visualização local (OpenCV)
- ✅ Visualização web (Flask)
- ✅ Controle remoto (YOLO on/off, restart)
- ✅ Status em tempo real
- ✅ Conexão segura (TLS)

## 🏗️ Arquitetura

```
┌─────────────┐         ┌──────────────┐         ┌─────────────┐
│  ESP32-CAM  │ ──MQTT──>│ HiveMQ Cloud │ ──MQTT──>│   Cliente   │
│  (Câmera)   │         │  (Broker)    │         │   Python    │
└─────────────┘         └──────────────┘         └─────────────┘
     │                          │                        │
     │                          │                        │
  Local WiFi              Internet (TLS)            Local/Web
```

### Tópicos MQTT

- `esp32cam/frames` - Frames da câmera (publicação)
- `esp32cam/status` - Status do sistema (publicação)
- `esp32cam/commands` - Comandos remotos (subscrição)

## ☁️ Configuração do HiveMQ Cloud

### Passo 1: Criar Conta

1. Acesse: https://www.hivemq.com/mqtt-cloud-broker/
2. Clique em "Start Free" ou "Sign Up"
3. Preencha o formulário de registro

### Passo 2: Criar Cluster

1. Após login, vá em "Clusters"
2. Clique em "Create Cluster"
3. Escolha uma região (ex: EU)
4. Anote o **Cluster URL** (ex: `abc123.s1.eu.hivemq.cloud`)

### Passo 3: Criar Credenciais

1. Vá em "Access Management" > "Credentials"
2. Clique em "Create New Credential Set"
3. Defina:
   - **Username**: Seu nome de usuário
   - **Password**: Sua senha (anote bem!)
4. Salve as credenciais

### Passo 4: Obter Informações

Você precisará de:
- **Broker URL**: `seu-cluster.s1.eu.hivemq.cloud`
- **Porta**: `8883` (TLS)
- **Username**: Seu username
- **Password**: Sua senha

## 🔧 Configuração da ESP32-CAM

### 1. Instalar Bibliotecas

No Arduino IDE ou via Arduino CLI:

```bash
arduino-cli lib install "PubSubClient"
arduino-cli lib install "ArduinoJson"
```

### 2. Configurar Credenciais

Abra `Esp32S-CAM.ino` e edite as seguintes linhas:

```cpp
// Configuração MQTT (HiveMQ Cloud)
const char *MQTT_BROKER = "SEU-CLUSTER.s1.eu.hivemq.cloud";  // Substitua!
const char *MQTT_USER = "SEU-USUARIO";                        // Substitua!
const char *MQTT_PASS = "SUA-SENHA";                          // Substitua!
```

### 3. Configurar Wi-Fi

```cpp
const char *WIFI_SSID = "Sua-Rede-WiFi";
const char *WIFI_PASS = "Sua-Senha-WiFi";
```

### 4. Compilar e Fazer Upload

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 Esp32S-CAM/Esp32S-CAM.ino
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32 Esp32S-CAM/Esp32S-CAM.ino
```

**Nota**: Substitua `COM3` pela porta serial correta.

### 5. Verificar Serial Monitor

Abra o Serial Monitor (115200 baud) e verifique:

```
[MQTT] Configurado. Broker: seu-cluster.s1.eu.hivemq.cloud
[MQTT] Tentando conectar ao broker...
[MQTT] Conectado!
[MQTT] Inscrito em: esp32cam/commands
```

## 📦 Instalação de Dependências

### Para Visualização Local (mqtt_viewer.py)

```bash
pip install paho-mqtt opencv-python numpy
```

### Para Visualização Web (mqtt_web_viewer.py)

```bash
pip install paho-mqtt opencv-python numpy flask flask-cors
```

## 🚀 Uso

### Opção 1: Visualização Local (OpenCV)

```bash
python mqtt_viewer.py
```

**Comandos:**
- `q` - Sair
- `y` - Toggle YOLO
- `m` - Toggle MQTT
- `r` - Reiniciar ESP32

### Opção 2: Visualização Web (Flask)

```bash
python mqtt_web_viewer.py
```

Acesse: **http://localhost:5000**

**Funcionalidades:**
- Streaming em tempo real
- Controles remotos (botões)
- Informações do sistema
- Interface moderna e responsiva

### Configurar Credenciais nos Scripts Python

Edite `mqtt_viewer.py` ou `mqtt_web_viewer.py`:

```python
MQTT_BROKER = "seu-cluster.s1.eu.hivemq.cloud"
MQTT_USER = "seu-usuario"
MQTT_PASS = "sua-senha"
```

## 🔍 Troubleshooting

### ESP32 não conecta ao MQTT

**Sintoma**: `[MQTT] Falha, rc=-2` ou `rc=-1`

**Soluções:**
1. Verifique se as credenciais estão corretas
2. Verifique se o Wi-Fi está conectado
3. Verifique se o broker URL está correto
4. Verifique se a porta é `8883` (TLS)

### Frames não aparecem

**Sintoma**: Cliente conecta mas não recebe frames

**Soluções:**
1. Verifique se o ESP32 está publicando frames:
   ```
   [MQTT] Frame publicado: 12345 bytes
   ```
2. Verifique se está inscrito no tópico correto: `esp32cam/frames`
3. Verifique o Serial Monitor do ESP32

### Erro de memória no ESP32

**Sintoma**: `[MQTT] Frame muito grande, pulando...`

**Soluções:**
1. Reduza a qualidade JPEG:
   ```cpp
   const int MQTT_JPEG_QUALITY = 15;  // Menor = melhor qualidade, maior tamanho
   ```
2. Aumente o intervalo de publicação:
   ```cpp
   const unsigned long MQTT_PUBLISH_INTERVAL = 2000;  // 2 segundos
   ```

### Erro de certificado TLS

**Sintoma**: Falha na conexão TLS

**Solução**: O código usa `setInsecure()` para desenvolvimento. Para produção, configure certificados adequados.

### Cliente Python não conecta

**Sintoma**: `[MQTT] Falha na conexão. Código: X`

**Códigos de erro:**
- `1` - Protocolo incorreto
- `2` - Client ID inválido
- `3` - Servidor indisponível
- `4` - Credenciais inválidas
- `5` - Não autorizado

**Soluções:**
1. Verifique credenciais (username/password)
2. Verifique se o broker URL está correto
3. Verifique se está usando porta `8883` (TLS)

## 📊 Limites do HiveMQ Cloud (Plano Gratuito)

- **100 conexões simultâneas**
- **10 MB de mensagens por mês**
- **Sem limite de tópicos**
- **TLS/SSL incluído**

**Nota**: Para frames grandes, considere:
- Reduzir qualidade JPEG
- Reduzir FPS (frames por segundo)
- Comprimir frames antes de enviar

## 🔐 Segurança

### Recomendações

1. **Não compartilhe credenciais** publicamente
2. Use senhas fortes
3. Para produção, configure certificados TLS adequados
4. Considere autenticação adicional na aplicação

### Melhorias Futuras

- [ ] Autenticação JWT
- [ ] Criptografia de payload
- [ ] Rate limiting
- [ ] Compressão de frames (JPEG → WebP)

## 📝 Estrutura de Dados

### Frame JSON

```json
{
  "timestamp": 1234567890,
  "frame_id": 42,
  "format": "jpeg",
  "width": 640,
  "height": 480,
  "size": 12345,
  "quality": 20,
  "data": "base64_encoded_jpeg_data..."
}
```

### Status JSON

```json
{
  "timestamp": 1234567890,
  "status": "online",
  "ip": "192.168.1.100",
  "uptime": 3600
}
```

### Comando JSON

```json
{
  "action": "toggle_yolo",
  "enabled": true
}
```

## 🎓 Conceitos Aprendidos

- **MQTT**: Protocolo de mensageria para IoT
- **HiveMQ Cloud**: Broker MQTT gerenciado
- **TLS/SSL**: Criptografia de transporte
- **Base64**: Codificação de dados binários
- **JSON**: Formato de dados estruturado
- **Flask**: Framework web Python
- **OpenCV**: Processamento de imagens

## 📚 Referências

- [HiveMQ Cloud](https://www.hivemq.com/mqtt-cloud-broker/)
- [MQTT Protocol](https://mqtt.org/)
- [PubSubClient Library](https://github.com/knolleary/pubsubclient)
- [ArduinoJson](https://arduinojson.org/)
- [Flask Documentation](https://flask.palletsprojects.com/)

## 📄 Licença

Este projeto é fornecido como está, para fins educacionais.

---

**Desenvolvido para o projeto M4-IOT**

