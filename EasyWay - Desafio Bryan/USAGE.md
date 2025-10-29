# EasyWay - Guia de Uso Completo

## 🚀 Início Rápido

### 1. Preparação do Hardware

#### Servidor:
- ESP32-WROOM-32U
- Módulo RF24 (NRF24L01+)
- LED + Resistor 220Ω
- Jumpers

#### Cliente:
- ESP32-WROOM-32U
- Módulo RF24 (NRF24L01+)
- 3 LEDs (Verde, Vermelho, Amarelo) + Resistores 220Ω
- Relé para porta
- Botão + Resistor 10kΩ
- Jumpers

### 2. Conexões

#### RF24 para ESP32:
```
RF24 VCC  → ESP32 3.3V
RF24 GND  → ESP32 GND
RF24 CE   → ESP32 GPIO 4
RF24 CSN  → ESP32 GPIO 5
RF24 MOSI → ESP32 GPIO 23
RF24 MISO → ESP32 GPIO 19
RF24 SCK  → ESP32 GPIO 18
```

#### Servidor - LED Status:
```
LED Anodo  → ESP32 GPIO 2
LED Catodo → Resistor 220Ω → GND
```

#### Cliente - LEDs:
```
LED Verde  → ESP32 GPIO 2
LED Vermelho → ESP32 GPIO 3
LED Amarelo → ESP32 GPIO 4
(Cada LED: Anodo → GPIO | Catodo → Resistor 220Ω → GND)
```

#### Cliente - Relé e Botão:
```
Relé VCC → 5V (ou 3.3V)
Relé GND → GND
Relé IN  → ESP32 GPIO 6

Botão Terminal 1 → ESP32 GPIO 7
Botão Terminal 2 → GND
Resistor 10kΩ → GPIO 7 → 3.3V (pull-up)
```

## 💻 Instalação do Software

### 1. Arduino IDE Setup

1. **Instalar ESP32 Board Package:**
   - File → Preferences
   - Adicionar URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager
   - Instalar "ESP32 by Espressif Systems"

2. **Instalar Bibliotecas:**
   - Tools → Manage Libraries
   - Instalar: ArduinoJson, CRC32

### 2. Configuração da Placa

```
Board: ESP32 Dev Module
Upload Speed: 115200
CPU Frequency: 240MHz
Flash Frequency: 80MHz
Flash Mode: QIO
Flash Size: 4MB
Partition Scheme: Default 4MB with spiffs (servidor)
PSRAM: Disabled
```

## 🔧 Configuração dos Códigos

### 1. Servidor (EasyWay_Server.ino)

**Não precisa alterar nada** - está pronto para uso!

### 2. Cliente (EasyWay_Client.ino)

**IMPORTANTE**: Altere o ID para cada cliente:

```cpp
const String DEVICE_ID = "cliente01"; // Altere para cliente02, cliente03, etc.
```

## 📤 Upload dos Códigos

### Servidor:

1. **Upload do Código:**
   - Seguir tutorial do README principal do repositório.

2. **Upload dos Arquivos Web:**
   - Tools → ESP32 Sketch Data Upload
   - Selecionar pasta `data/` com arquivos HTML/CSS/JS

### Cliente:

1. **Upload do Código:**
   - Abrir `EasyWay_Client.ino`
   - Alterar `DEVICE_ID`
   - Compilar (`Ctrl+R`)
   - Upload (`Ctrl+U`)

## 🚀 Inicialização do Sistema

### 1. Sequência de Ligamento:

1. **Ligar o Servidor primeiro**
2. **Aguardar 30 segundos**
3. **Ligar os Clientes**

### 2. Verificação Inicial:

#### Servidor (Serial Monitor 115200):
```
EasyWay Server - Iniciando sistema...
Comunicação via rádio inicializada com sucesso
Camada de segurança inicializada
Gerenciador de dispositivos inicializado
Servidor web inicializado com sucesso
Acesse: http://192.168.4.1
```

#### Cliente (Serial Monitor 115200):
```
EasyWay Client - Iniciando dispositivo...
Comunicação via rádio inicializada com sucesso
Camada de segurança inicializada
Gerenciador de LEDs inicializado
Controlador de dispositivo inicializado
EasyWay Client - Dispositivo inicializado!
ID do dispositivo: cliente01
```

### 3. LEDs de Status:

#### Servidor:
- **LED Ligado**: Sistema funcionando

#### Cliente:
- **LED Verde Ligado**: Conectado ao servidor
- **LED Vermelho Ligado**: Erro de comunicação
- **LED Amarelo Piscando**: Transmitindo dados

## 🌐 Acesso à Interface Web

### 1. Conectar à Rede WiFi:

- **SSID**: EasyWay-Server
- **Senha**: easyway123

### 2. Abrir Interface:

- **URL**: http://192.168.4.1
- **Navegador**: Qualquer navegador moderno

### 3. Interface Disponível:

- **Dispositivos**: Ver clientes conectados
- **Controle**: Enviar comandos
- **Mensagens**: Trocar mensagens
- **Logs**: Histórico do sistema

## 🎮 Como Usar

### 1. Controle Remoto de Portas:

1. **Acesse a aba "Controle"**
2. **Selecione um dispositivo**
3. **Clique em "Abrir Porta" ou "Fechar Porta"**
4. **Aguarde confirmação**

### 2. Envio de Mensagens:

1. **Acesse a aba "Controle"**
2. **Selecione um dispositivo**
3. **Digite a mensagem**
4. **Clique em "Enviar"**

### 3. Monitoramento:

1. **Aba "Dispositivos"**: Status em tempo real
2. **Aba "Logs"**: Histórico de atividades
3. **Aba "Mensagens"**: Mensagens trocadas

### 4. Controle Local (Botão):

- **Pressionar botão**: Alterna estado da porta
- **LEDs indicam**: Status da operação

## 🔍 Solução de Problemas

### Problema: Cliente não conecta

**Sintomas:**
- LED vermelho ligado
- Serial mostra "Timeout detectado"

**Soluções:**
1. Verificar se servidor está ligado
2. Verificar conexões do RF24
3. Verificar alimentação (3.3V)
4. Testar com outro módulo RF24

### Problema: Interface web não carrega

**Sintomas:**
- Página em branco
- Erro de conexão

**Soluções:**
1. Verificar conexão WiFi
2. Testar IP: 192.168.4.1
3. Limpar cache do navegador
4. Verificar upload dos arquivos SPIFFS

### Problema: Comandos não funcionam

**Sintomas:**
- Comando enviado mas porta não abre/fecha
- Erro na interface web

**Soluções:**
1. Verificar se dispositivo está online
2. Verificar conexões do relé
3. Testar comando local (botão)
4. Verificar logs do sistema

### Problema: LEDs não funcionam

**Sintomas:**
- LEDs não acendem
- LEDs ficam sempre ligados

**Soluções:**
1. Verificar conexões dos LEDs
2. Verificar resistores
3. Testar com multímetro
4. Verificar código de inicialização

## 📊 Monitoramento Avançado

### Serial Monitor:

**Configurações:**
- Baud Rate: 115200
- Line Ending: Both NL & CR
- Auto-scroll: Enabled

**Logs Importantes:**
- Mensagens de inicialização
- Comandos enviados/recebidos
- Erros de comunicação
- Status dos dispositivos

### Interface Web:

**Aba Logs:**
- Histórico completo de atividades
- Timestamps precisos
- Detalhes de cada operação

**Aba Dispositivos:**
- Status em tempo real
- Última atividade
- Comandos executados

## 🔧 Manutenção

### Limpeza de Logs:

1. **Via Interface Web:**
   - Aba "Logs" → Botão "Limpar"

2. **Via Código:**
   - Reiniciar servidor limpa logs

### Atualização de Firmware:

1. **Servidor:**
   - Modificar código
   - Compilar e upload
   - Upload arquivos SPIFFS

2. **Cliente:**
   - Modificar código
   - Compilar e upload
   - Reiniciar dispositivo

### Backup de Configurações:

1. **Salvar códigos modificados**
2. **Documentar IDs dos dispositivos**
3. **Registrar configurações de rede**

## 🚀 Expansões Futuras

### Adicionar Mais Clientes:

1. **Copiar pasta do cliente**
2. **Alterar DEVICE_ID único**
3. **Compilar e upload**
4. **Testar comunicação**

### Adicionar Sensores:

1. **Conectar sensor ao ESP32**
2. **Modificar código do cliente**
3. **Adicionar tipo de mensagem**
4. **Atualizar interface web**

### Personalizar Interface:

1. **Modificar arquivos HTML/CSS/JS**
2. **Upload para SPIFFS**
3. **Reiniciar servidor**

## 📞 Suporte

### Informações de Debug:

**Servidor:**
- Serial Monitor: Logs detalhados
- Interface Web: Status e logs
- LED: Indica funcionamento

**Cliente:**
- Serial Monitor: Operações do dispositivo
- LEDs: Status visual
- Botão: Teste local

### Logs Importantes:

**Inicialização:**
- Confirmação de bibliotecas
- Status de conexões
- Configurações aplicadas

**Operação:**
- Comandos enviados/recebidos
- Confirmações ACK
- Erros de comunicação

**Problemas:**
- Timeouts
- Falhas de validação
- Erros de hardware

## ✅ Checklist Final

### Antes de Usar:
- [ ] Hardware conectado corretamente
- [ ] Códigos compilados sem erro
- [ ] IDs únicos configurados
- [ ] Arquivos web enviados para SPIFFS
- [ ] Servidor ligado primeiro
- [ ] Clientes ligados após servidor
- [ ] LEDs indicando status correto
- [ ] Interface web carregando
- [ ] Comandos funcionando
- [ ] Mensagens sendo trocadas

### Para Produção:
- [ ] Senhas alteradas
- [ ] IDs não óbvios
- [ ] Debug desabilitado
- [ ] Logs monitorados
- [ ] Backup realizado
- [ ] Documentação atualizada
