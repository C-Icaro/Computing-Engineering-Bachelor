# EasyWay Server - Configuração e Instalação

## 📋 Pré-requisitos

### Hardware Necessário:
- ESP32-WROOM-32U (com antena externa IPEX)
- LED de status
- Resistor 220Ω
- Jumpers e protoboard

### Software Necessário:
- Arduino IDE 2.0+
- ESP32 Board Package
- Bibliotecas listadas abaixo

## 📚 Bibliotecas Necessárias

Instale as seguintes bibliotecas através do Library Manager do Arduino IDE:

```cpp

// Servidor web e WiFi
#include <WiFi.h>           // Incluída no ESP32 Core
#include <WebServer.h>     // Incluída no ESP32 Core
#include <SPIFFS.h>        // Incluída no ESP32 Core

// JSON e segurança
#include <ArduinoJson.h>   // Versão 6.21.3+
#include <CRC32.h>         // Versão 1.0.0+
```

### Como Instalar as Bibliotecas:

1. Abra o Arduino IDE
2. Vá em `Tools` → `Manage Libraries...`
3. Procure por cada biblioteca e clique em `Install`

## 🔌 Conexões do Hardware

### Módulo RF24 (NRF24L01+):
```
RF24    →    ESP32
VCC     →    3.3V
GND     →    GND
CE      →    GPIO 4
CSN     →    GPIO 5
MOSI    →    GPIO 23
MISO    →    GPIO 19
SCK     →    GPIO 18
```

### LED de Status:
```
LED Anodo    →    GPIO 2
LED Catodo   →    Resistor 220Ω → GND
```

## ⚙️ Configuração do Arduino IDE

### 1. Instalar ESP32 Board Package:

1. Abra o Arduino IDE
2. Vá em `File` → `Preferences`
3. Em `Additional Board Manager URLs`, adicione:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Vá em `Tools` → `Board` → `Boards Manager...`
5. Procure por "ESP32" e instale o pacote oficial

### 2. Configurar a Placa:

1. Vá em `Tools` → `Board` → `ESP32 Arduino` → `ESP32 Dev Module`
2. Configure as seguintes opções:
   - **Upload Speed**: 115200
   - **CPU Frequency**: 240MHz
   - **Flash Frequency**: 80MHz
   - **Flash Mode**: QIO
   - **Flash Size**: 4MB
   - **Partition Scheme**: Default 4MB with spiffs
   - **PSRAM**: Disabled

## 📁 Upload dos Arquivos

### 1. Compilar e Carregar o Código:

1. Abra o arquivo `EasyWay_Server.ino`
2. Verifique se todas as bibliotecas estão instaladas
3. Compile o código (`Ctrl+R`)
4. Carregue na ESP32 (`Ctrl+U`)

### 2. Upload dos Arquivos Web para SPIFFS:

#### Opção A - Usando ESP32 Sketch Data Upload:

1. Instale o plugin: `Tools` → `Manage Libraries...` → Procure "ESP32 Sketch Data Upload"
2. Após carregar o código principal, vá em `Tools` → `ESP32 Sketch Data Upload`
3. Selecione a pasta `data/` que contém os arquivos HTML, CSS e JS

#### Opção B - Usando Arduino IDE com SPIFFS:

1. Instale a ferramenta SPIFFS Data Upload
2. Coloque os arquivos da pasta `data/` na pasta `data/` do sketch
3. Use `Tools` → `ESP32 Sketch Data Upload`

## 🔧 Configurações do Sistema

### Configurações de Rede (modificáveis no código):

```cpp
const char* WIFI_SSID = "EasyWay-Server";     // Nome da rede WiFi
const char* WIFI_PASSWORD = "easyway123";     // Senha da rede
```

### Configurações de Rádio (modificáveis no código):

```cpp
const int RADIO_CE_PIN = 4;    // Pino CE do RF24
const int RADIO_CSN_PIN = 5;    // Pino CSN do RF24
const int LED_STATUS_PIN = 2;   // Pino do LED de status
```

### Configurações de Canal RF24:

```cpp
radio.setChannel(76);           // Canal 76 (2.476 GHz)
radio.setPALevel(RF24_PA_MAX); // Potência máxima
radio.setDataRate(RF24_250KBPS); // Taxa de dados 250kbps
```

## 🚀 Teste e Verificação

### 1. Verificação Inicial:

1. Abra o Serial Monitor (115200 baud)
2. Reinicie a ESP32
3. Verifique as mensagens de inicialização:
   ```
   EasyWay Server - Iniciando sistema...
   Comunicação via rádio inicializada com sucesso
   Servidor web inicializado com sucesso
   Acesse: http://192.168.4.1
   ```

### 2. Teste da Rede WiFi:

1. Procure pela rede "EasyWay-Server" nos dispositivos WiFi
2. Conecte-se usando a senha "easyway123"
3. Abra um navegador e acesse `http://192.168.4.1`

### 3. Teste da Interface Web:

1. A página principal deve carregar
2. Verifique se todas as abas funcionam
3. Teste a aba "Dispositivos" (deve mostrar "Nenhum dispositivo conectado")

## 🐛 Solução de Problemas

### Problema: "Falha ao inicializar comunicação via rádio"

**Soluções:**
- Verifique as conexões do módulo RF24
- Confirme se está usando 3.3V (não 5V)
- Teste com outro módulo RF24
- Verifique se os pinos estão corretos

### Problema: "Falha ao inicializar servidor web"

**Soluções:**
- Verifique se os arquivos foram enviados para o SPIFFS
- Confirme a configuração da placa ESP32
- Teste com uma ESP32 diferente
- Verifique se há conflito de pinos

### Problema: Interface web não carrega

**Soluções:**
- Verifique se está conectado à rede "EasyWay-Server"
- Teste o IP: `http://192.168.4.1`
- Limpe o cache do navegador
- Teste em outro dispositivo

### Problema: Página em branco

**Soluções:**
- Verifique se os arquivos HTML/CSS/JS estão no SPIFFS
- Confirme o upload dos arquivos da pasta `data/`
- Verifique o console do navegador para erros

## 📊 Monitoramento

### Serial Monitor:
- Baud rate: 115200
- Mostra logs detalhados de todas as operações
- Útil para debug e monitoramento

### Logs da Interface Web:
- Acesse a aba "Logs" na interface web
- Mostra histórico de comandos e atividades
- Atualiza em tempo real

## 🔄 Atualizações

### Para Atualizar o Código:
1. Modifique os arquivos `.ino`, `.cpp` ou `.h`
2. Compile e carregue novamente

### Para Atualizar a Interface Web:
1. Modifique os arquivos HTML/CSS/JS
2. Faça upload novamente para o SPIFFS
3. Reinicie a ESP32

## 📈 Otimizações

### Performance:
- Use `RF24_PA_MAX` para maior alcance
- Configure `RF24_250KBPS` para melhor confiabilidade
- Ajuste o intervalo de heartbeat conforme necessário

### Consumo de Energia:
- Implemente modo sleep nos clientes
- Use transmissão apenas quando necessário
- Configure timeouts apropriados

## 🔐 Segurança

### Configurações de Segurança:
- Altere a senha WiFi padrão
- Modifique a chave secreta em `SecurityLayer.cpp`
- Configure IDs únicos para cada dispositivo
- Use checksums CRC32 para validação

### Boas Práticas:
- Mantenha os dispositivos atualizados
- Monitore os logs regularmente
- Use senhas fortes
- Limite o acesso físico aos dispositivos
