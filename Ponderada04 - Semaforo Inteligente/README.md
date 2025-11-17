# Ponderada 04 - Semáforo Inteligente

## 📋 Informações Gerais

**Disciplina:** M4-IOT  
**Professor:** Bryan Kano  
**Objetivo:** Criar um semáforo inteligente capaz de detectar condições de luminosidade através de um sensor LDR e adaptar seu comportamento automaticamente, com interface web e integração MQTT para Smart Cities.

## 🎯 Objetivos da Atividade

1. **Montar** dois semáforos físicos com LEDs (vermelho, amarelo, verde)
2. **Integrar** sensor LDR para detecção de luminosidade
3. **Implementar** modo noturno automático baseado em histerese
4. **Criar** interface web para controle e monitoramento
5. **Integrar** broker MQTT local (Mosquitto) para comunicação IoT
6. **Documentar** o funcionamento completo do sistema

## 🔧 Hardware Necessário

### Componentes

| Componente | Quantidade | Especificação |
|------------|------------|---------------|
| ESP32 (ESP-WROOM-32) | 1 | DevKit ou similar |
| LED vermelho | 2 | 5 mm (ou similar) |
| LED amarelo | 2 | 5 mm (ou similar) |
| LED verde | 2 | 5 mm (ou similar) |
| LDR (Light Dependent Resistor) | 1 | Sensor de luz |
| Resistor fixo | 1 | 10 kΩ (para divisor de tensão do LDR) |
| Resistores para LEDs | 6 | 220-330 Ω, 1/4 W |
| Protoboard | 1 | 400-830 pontos |
| Jumpers | vários | macho-macho |

### Mapeamento de Pinos

#### Semáforo 1 (S1)
- **LED Vermelho:** Pino 27
- **LED Amarelo:** Pino 14
- **LED Verde:** Pino 12

#### Semáforo 2 (S2)
- **LED Vermelho:** Pino 33
- **LED Amarelo:** Pino 25
- **LED Verde:** Pino 26

#### Sensor LDR
- **LDR:** Pino 32 (ADC)
- **Divisor de tensão:** LDR + Resistor 10 kΩ para GND

## 📐 Montagem Física

### Circuito do LDR

O LDR deve ser conectado em um divisor de tensão:

1. **Terminal 1 do LDR** → 3.3V (ou 5V)
2. **Terminal 2 do LDR** → Pino 32 (ADC) + Resistor 10 kΩ → GND

**Nota:** O resistor de 10 kΩ deve estar entre o pino 32 e o GND, formando o divisor de tensão.

### Circuito dos LEDs

Para cada LED:
1. **Ânodo (perna longa)** → Resistor 220-330 Ω → Pino do ESP32
2. **Cátodo (perna curta)** → GND

### Diagrama de Conexão

```
ESP32                    Componentes
------                    -----------
3.3V  ────────────────┬── LDR ──── Pino 32
                       │
                       └── Resistor 10kΩ ──── GND

Pino 27 ─── Resistor ─── LED Vermelho S1 ─── GND
Pino 14 ─── Resistor ─── LED Amarelo S1 ─── GND
Pino 12 ─── Resistor ─── LED Verde S1 ─── GND
Pino 33 ─── Resistor ─── LED Vermelho S2 ─── GND
Pino 25 ─── Resistor ─── LED Amarelo S2 ─── GND
Pino 26 ─── Resistor ─── LED Verde S2 ─── GND
```

## 💻 Configuração do Projeto

### 1. Pré-requisitos

- **Arduino CLI** instalado e configurado
- **Plataforma ESP32** instalada no Arduino CLI
- **Biblioteca PubSubClient** instalada

### 2. Instalação das Dependências

#### Instalar plataforma ESP32

```powershell
arduino-cli core install esp32:esp32
```

#### Instalar biblioteca PubSubClient

```powershell
arduino-cli lib install "PubSubClient"
```

#### Verificar instalações

```powershell
# Verificar plataforma
arduino-cli core list

# Verificar bibliotecas
arduino-cli lib list
```

### 3. Configuração do Código

#### Ajustar credenciais Wi-Fi

Edite as linhas 5-6 do arquivo `.ino`:

```cpp
const char* ssid = "Nome_rede";        // Nome da sua rede Wi-Fi
const char* password = "Senha_rede";   // Senha da rede
```

**Importante:** O ESP32 criará um Access Point com essas credenciais. Certifique-se de que não conflitam com redes existentes.

#### Ajustar IP do Broker MQTT

1. **Conecte seu PC à rede Wi-Fi criada pelo ESP32** (mesma rede configurada acima)

2. **Descubra o IP do seu PC:**
   - **Windows:** Abra CMD e execute `ipconfig`
   - **Linux/Mac:** Execute `ifconfig` no terminal
   - Procure pelo **Endereço IPv4** da interface Wi-Fi conectada

3. **Edite a linha 13** do arquivo `.ino`:

```cpp
const char* mqtt_server = "192.168.4.2";  // Use o IP do seu PC
```

**Exemplo:** Se o `ipconfig` mostrar `192.168.4.5`, use:

```cpp
const char* mqtt_server = "192.168.4.5";
```

## 🔨 Compilação

### Compilar o projeto

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32 "Ponderada04 - Semaforo Inteligente"
```

### Verificar erros

Se houver erros de compilação:

1. **Biblioteca não encontrada:** Instale com `arduino-cli lib install "PubSubClient"`
2. **Plataforma não encontrada:** Instale com `arduino-cli core install esp32:esp32`
3. **Erro de sintaxe:** Verifique o código no editor

## 📤 Upload para o ESP32

### 1. Identificar a porta do ESP32

```powershell
arduino-cli board list
```

Procure pela porta COM (ex: `COM3`, `COM5`, etc.)

### 2. Fazer upload

```powershell
arduino-cli upload -p COM5 --fqbn esp32:esp32:esp32 "Ponderada04 - Semaforo Inteligente"
```

**Substitua `COM5` pela porta do seu ESP32.**

### 3. Verificar upload

Após o upload, abra o Serial Monitor:

```powershell
arduino-cli monitor -p COM5 -c baudrate=115200
```

Você deve ver mensagens como:

```
========================================
  SEMAFORO INTELIGENTE - INICIANDO
========================================

[Setup] Inicializando controlador...
[Setup] AP criado com sucesso! SSID: iPhone
[Setup] IP do Access Point: 192.168.4.1
[Setup] Servidor HTTP iniciado com sucesso!
```

## 🌐 Interface Web (WebServer)

### Acessar a Interface

1. **Conecte seu dispositivo** (notebook, celular, tablet) à rede Wi-Fi criada pelo ESP32
   - **SSID:** O nome configurado (ex: "iPhone")
   - **Senha:** A senha configurada (ex: "12345678")

2. **Abra o navegador** e acesse:

```
http://192.168.4.1
```

**Nota:** O IP padrão do ESP32 como Access Point é `192.168.4.1`. Verifique no Serial Monitor se for diferente.

### Funcionalidades da Interface

#### Dashboard Principal

- **Luminosidade em tempo real:** Valor atual do sensor LDR (0-5000)
- **Barra de progresso:** Visualização gráfica do nível de luminosidade
- **Modo atual:** Exibe o modo ativo (Automático, Normal ou Noturno)
- **Badges de status:** Indicadores visuais dos modos disponíveis
- **Visualização dos semáforos:** Representação visual dos dois semáforos

#### Controles

- **🤖 Modo Automático:** Ativa detecção automática baseada no LDR
- **☀️ Modo Normal:** Força ciclo completo do semáforo (ignora LDR)
- **🌙 Modo Noturno:** Força modo noturno (amarelo piscando)

#### Endpoint JSON

Acesse para obter dados em formato JSON:

```
http://192.168.4.1/status
```

**Resposta exemplo:**

```json
{
  "luminosidade": 1450,
  "modoAuto": true,
  "modoNoturno": false,
  "timestamp": 12345678
}
```

### Atualização Automática

A interface atualiza automaticamente a cada 2 segundos via JavaScript, mostrando valores em tempo real sem necessidade de recarregar a página.

## 📡 Instalação e Configuração do MQTT (Mosquitto)

### 1. Instalar Mosquitto

#### Windows

1. Baixe o instalador em: https://mosquitto.org/download/
2. Execute o instalador e siga as instruções
3. O serviço Mosquitto será instalado automaticamente

**Verificar instalação:**

```powershell
mosquitto --version
```

#### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install mosquitto mosquitto-clients
```

#### macOS

```bash
brew install mosquitto
```

### 2. Iniciar o Broker

#### Windows

O serviço inicia automaticamente após a instalação. Para gerenciar:

```powershell
# Verificar status
Get-Service mosquitto

# Iniciar (se necessário)
Start-Service mosquitto

# Parar
Stop-Service mosquitto
```

#### Linux

```bash
# Iniciar serviço
sudo systemctl start mosquitto

# Habilitar no boot
sudo systemctl enable mosquitto

# Verificar status
sudo systemctl status mosquitto
```

#### macOS

```bash
# Iniciar
brew services start mosquitto
```

### 3. Configurar Firewall (se necessário)

O Mosquitto usa a porta **1883** por padrão. Certifique-se de que ela está aberta:

#### Windows

```powershell
# Permitir porta 1883
New-NetFirewallRule -DisplayName "Mosquitto MQTT" -Direction Inbound -LocalPort 1883 -Protocol TCP -Action Allow
```

#### Linux

```bash
sudo ufw allow 1883/tcp
```

### 4. Testar o Broker

#### Publicar uma mensagem

```powershell
# Windows/Linux/Mac
mosquitto_pub -h localhost -t "test/topic" -m "Hello MQTT"
```

#### Subscrever a um tópico

```powershell
# Windows/Linux/Mac
mosquitto_sub -h localhost -t "test/topic"
```

Se você conseguir publicar e receber mensagens, o broker está funcionando!

### 5. Inicialização e Configuração Detalhada do Broker

#### Verificar se o Mosquitto está instalado corretamente

```powershell
# Verificar versão
mosquitto --version

# Verificar se o executável está no PATH
where mosquitto
```

#### Problemas Comuns na Inicialização

##### Problema: Serviço não inicia no Windows

**Sintoma:** Erro "Não é possível abrir o serviço mosquitto"

**Soluções:**

1. **Executar PowerShell como Administrador:**
   - Clique com botão direito no PowerShell
   - Selecione "Executar como administrador"
   - Tente novamente: `Start-Service mosquitto`

2. **Verificar se o serviço existe:**
   ```powershell
   Get-Service | Where-Object {$_.Name -like "*mosquitto*"}
   ```

3. **Se o serviço não existir, executar manualmente:**
   ```powershell
   # Navegar até a pasta de instalação (geralmente)
   cd "C:\Program Files\mosquitto"
   
   # Executar o broker manualmente
   .\mosquitto.exe -c mosquitto.conf
   ```

4. **Reinstalar o Mosquitto:**
   - Desinstale completamente
   - Baixe a versão mais recente
   - Reinstale com privilégios de administrador

##### Problema: IP APIPA (169.254.x.x)

**Sintoma:** O PC recebe um IP `169.254.x.x` ao conectar na rede do ESP32

**Causa:** O Windows não conseguiu obter IP via DHCP

**Solução - Configurar IP Estático:**

1. **Conecte o PC à rede Wi-Fi do ESP32**

2. **Abra as Configurações de Rede:**
   - Windows: Configurações → Rede e Internet → Wi-Fi → Gerenciar redes conhecidas
   - Ou: Painel de Controle → Centro de Rede e Compartilhamento

3. **Configure IP Estático:**
   - Clique com botão direito na rede do ESP32
   - Propriedades → Protocolo IP versão 4 (TCP/IPv4)
   - Selecione "Usar o seguinte endereço IP"
   - Configure:
     - **Endereço IP:** `192.168.4.2` (ou outro disponível)
     - **Máscara de sub-rede:** `255.255.255.0`
     - **Gateway padrão:** `192.168.4.1` (IP do ESP32)
   - Clique em OK

4. **Atualize o código com o IP estático:**
   ```cpp
   const char* mqtt_server = "192.168.4.2";  // IP estático configurado
   ```

##### Problema: Firewall bloqueando conexões

**Sintoma:** Broker funciona localmente mas ESP32 não conecta

**Solução:**

```powershell
# Permitir porta 1883 no firewall (executar como Admin)
New-NetFirewallRule -DisplayName "Mosquitto MQTT" -Direction Inbound -LocalPort 1883 -Protocol TCP -Action Allow

# Verificar regras criadas
Get-NetFirewallRule | Where-Object {$_.DisplayName -like "*Mosquitto*"}
```

##### Verificar se o broker está escutando na interface correta

Por padrão, o Mosquitto pode estar escutando apenas em `localhost`. Para aceitar conexões externas:

1. **Localizar arquivo de configuração:**
   - Windows: `C:\Program Files\mosquitto\mosquitto.conf`
   - Linux: `/etc/mosquitto/mosquitto.conf`

2. **Editar configuração:**
   ```conf
   # Permitir conexões de qualquer interface
   listener 1883 0.0.0.0
   
   # Ou permitir apenas da rede local
   listener 1883 192.168.4.0/24
   ```

3. **Reiniciar o serviço:**
   ```powershell
   Restart-Service mosquitto
   ```

#### Checklist de Inicialização

Antes de conectar o ESP32, verifique:

- [ ] Mosquitto está instalado e funcionando
- [ ] Serviço está rodando (`Get-Service mosquitto`)
- [ ] Broker responde localmente (`mosquitto_sub -h localhost -t "test"`)
- [ ] PC está conectado à rede Wi-Fi do ESP32
- [ ] PC tem IP válido na rede (não 169.254.x.x)
- [ ] IP do PC está configurado corretamente no código
- [ ] Firewall permite conexões na porta 1883
- [ ] Broker está escutando na interface correta (não apenas localhost)

## 📖 Principais Partes do Código

### Estrutura Geral do Projeto

O código está organizado em **Programação Orientada a Objetos (POO)** com as seguintes seções:

```
1. Inclusões e Configurações Globais
2. Classes (Semaforo, SemaforoInteligente)
3. Funções MQTT
4. Funções HTTP/WebServer
5. Setup e Loop
```

### 1. Configurações e Variáveis Globais

#### Wi-Fi Access Point (Linhas 4-7)

```cpp
const char* ssid = "Nome_rede";
const char* password = "Senha_rede";
WebServer server(80);
```

- **Função:** Configura o ESP32 como Access Point (cria uma rede Wi-Fi)
- **SSID/Password:** Credenciais da rede que o ESP32 criará
- **WebServer:** Servidor HTTP na porta 80 para interface web

#### Configuração MQTT (Linhas 8-21)

```cpp
const char* mqtt_server = "192.168.4.2";
const int mqtt_port = 1883;
const char* mqtt_client_id = "semaforo_inteligente";
const char* mqtt_topic_telemetria = "semaforo/telemetria";
const char* mqtt_topic_comandos = "semaforo/comandos";
```

- **mqtt_server:** IP do PC onde o Mosquitto está rodando
- **mqtt_port:** Porta padrão do MQTT (1883)
- **mqtt_client_id:** Identificador único do cliente MQTT
- **Tópicos:** Canais de comunicação (telemetria = dados, comandos = controle)

#### Mapeamento de Pinos (Linhas 22-30)

```cpp
const int S1_red = 27, S1_yellow = 14, S1_green = 12;
const int S2_red = 33, S2_yellow = 25, S2_green = 26;
const int LDR_PIN = 32;
```

- Define quais pinos do ESP32 controlam cada LED e o sensor LDR

### 2. Classe `Semaforo` (Linhas 36-64)

**Responsabilidade:** Controlar um semáforo individual (3 LEDs)

```cpp
class Semaforo {
  // Encapsula: pinos dos LEDs (vermelho, amarelo, verde)
  // Métodos principais:
  void verde()    // Acende apenas verde
  void amarelo()  // Acende apenas amarelo
  void vermelho() // Acende apenas vermelho
  void amareloPisca(bool ligado) // Pisca amarelo (modo noturno)
}
```

**Princípio POO:** Encapsulamento - cada semáforo gerencia seus próprios LEDs

### 3. Classe `SemaforoInteligente` (Linhas 66-218)

**Responsabilidade:** Lógica principal do sistema (controle dos dois semáforos + LDR)

#### Estrutura de Telemetria (Linhas 68-73)

```cpp
struct Telemetria {
  int luz;              // Valor do LDR (0-5000)
  bool autoAtivo;       // Modo automático ativo?
  bool noturnoAtivo;    // Modo noturno ativo?
  unsigned long timestamp; // Momento da leitura
};
```

#### Métodos Principais

**`begin()` (Linhas 80-89):**
- Inicializa os dois semáforos
- Configura o pino do LDR como entrada
- Inicializa timers

**`atualizar()` (Linhas 91-98):**
- **Função principal do loop:** Executada continuamente
- Lê luminosidade do LDR
- Aplica histerese se modo automático
- Escolhe entre ciclo normal ou noturno
- Atualiza telemetria e publica via MQTT

**`aplicarHisterese()` (Linhas 136-149):**
- **Histerese:** Evita oscilações frequentes
- Entra em modo noturno quando LDR < 1800
- Sai do modo noturno quando LDR > 2200
- Zona morta entre 1800-2200 mantém estado atual

**`cicloNormal()` (Linhas 152-164):**
- Máquina de estados não bloqueante
- 4 estados: S1 Verde → S1 Amarelo → S2 Verde → S2 Amarelo
- Usa `millis()` para temporização (sem `delay()`)

**`cicloNoturno()` (Linhas 172-180):**
- Ambos semáforos piscam amarelo simultaneamente
- Intervalo de 500ms (ligado/desligado)

### 4. Funções MQTT (Linhas 224-302)

#### `callbackMQTT()` (Linhas 226-250)

**Função:** Processa mensagens recebidas do broker

```cpp
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  // Recebe comandos no tópico "semaforo/comandos"
  // Comandos aceitos: "auto", "normal", "noturno"
  // Executa ação correspondente no controlador
}
```

**Fluxo:**
1. Broker recebe mensagem no tópico `semaforo/comandos`
2. Chama `callbackMQTT()` automaticamente
3. Interpreta comando e altera modo do semáforo

#### `reconectarMQTT()` (Linhas 252-271)

**Função:** Reconecta ao broker se perder conexão

```cpp
void reconectarMQTT() {
  while (!mqttClient.connected()) {
    // Tenta conectar
    // Se conseguir, subscreve ao tópico de comandos
    // Se falhar, aguarda 5 segundos e tenta novamente
  }
}
```

**Códigos de erro comuns:**
- `rc=-2`: Network unreachable (rede não alcançável)
- `rc=-1`: Connection refused (broker recusou)
- `rc=0`: Sucesso

#### `publicarTelemetriaMQTT()` (Linhas 273-302)

**Função:** Publica dados do semáforo no broker

```cpp
void publicarTelemetriaMQTT() {
  // Verifica conexão (reconecta se necessário)
  // A cada 5 segundos:
  //   - Cria JSON com telemetria atual
  //   - Publica no tópico "semaforo/telemetria"
}
```

**Formato JSON publicado:**
```json
{
  "luminosidade": 1450,
  "modoAuto": true,
  "modoNoturno": false,
  "timestamp": 12345678
}
```

### 5. Funções HTTP/WebServer (Linhas 304-592)

#### `handleRoot()` (Linhas 306-592)

**Função:** Gera e envia a interface web HTML

- Cria HTML completo com CSS e JavaScript embutidos
- Atualiza valores em tempo real via JavaScript
- Endpoints: `/`, `/auto`, `/normal`, `/noturno`, `/status`

#### `handleStatus()` (Linhas 594-603)

**Função:** Retorna dados em formato JSON

- Endpoint: `http://192.168.4.1/status`
- Usado pela interface web para atualização automática
- Mesmo formato do JSON publicado via MQTT

### 6. Setup e Loop

#### `setup()` (Linhas 707-771)

**Ordem de inicialização:**

1. **Serial Monitor** (115200 baud)
2. **Controlador** (inicializa semáforos e LDR)
3. **Wi-Fi AP** (cria rede Wi-Fi)
4. **Servidor HTTP** (configura rotas)
5. **Cliente MQTT** (tenta conectar ao broker)

**Importante:** O sistema continua funcionando mesmo se MQTT falhar

#### `loop()` (Linhas 775-787)

**Executado continuamente:**

```cpp
void loop() {
  server.handleClient();      // Processa requisições HTTP
  controlador.atualizar();     // Atualiza semáforos e LDR
  // Heartbeat a cada 10s
}
```

**Não bloqueante:** Usa `millis()` em vez de `delay()`, mantendo o sistema responsivo

### Fluxo de Dados

```
┌─────────────┐
│   Sensor    │
│    LDR      │───┐
└─────────────┘   │
                  │
┌─────────────┐   │   ┌──────────────────┐
│  ESP32      │◄──┴───│ SemaforoInteligente│
│  (Código)   │       │  (Classe)        │
└─────────────┘       └──────────────────┘
      │                        │
      │                        │
      ├────────────────────────┼──────────────┐
      │                        │              │
      ▼                        ▼              ▼
┌──────────┐          ┌─────────────┐  ┌──────────┐
│  Web     │          │    MQTT     │  │ Semáforos│
│ Server   │          │  (Mosquitto)│  │  (LEDs)  │
│ (HTTP)   │          │             │  │          │
└──────────┘          └─────────────┘  └──────────┘
      │                        │
      │                        │
      ▼                        ▼
┌──────────┐          ┌─────────────┐
│Interface │          │  Dashboard  │
│  Web     │          │  Externo    │
│(Browser) │          │  (Opcional) │
└──────────┘          └─────────────┘
```

### Conceitos POO Aplicados

1. **Encapsulamento:** Cada classe gerencia seus próprios dados
2. **Abstração:** `SemaforoInteligente` abstrai a complexidade do sistema
3. **Reutilização:** Classe `Semaforo` usada para ambos os semáforos
4. **Separação de responsabilidades:** Cada classe tem uma função específica

## 🔌 Uso do MQTT com o Semáforo

### 1. Verificar Conexão

Após fazer upload do código no ESP32, verifique no Serial Monitor se a conexão MQTT foi estabelecida:

```
[Setup] Conectado ao broker MQTT com sucesso!
[Setup] Inscrito no topico de comandos: semaforo/comandos
```

Se aparecer:

```
[Setup] AVISO: Nao foi possivel conectar ao broker MQTT.
```

Verifique:
- ✅ Mosquitto está rodando no PC
- ✅ PC está na mesma rede Wi-Fi do ESP32
- ✅ IP do broker está correto no código
- ✅ Firewall permite conexões na porta 1883

### 2. Subscrever à Telemetria

Para receber os dados do semáforo em tempo real:

```powershell
mosquitto_sub -h localhost -t "semaforo/telemetria"
```

**Saída esperada (a cada 5 segundos):**

```json
{"luminosidade":1450,"modoAuto":true,"modoNoturno":false,"timestamp":12345678}
{"luminosidade":1430,"modoAuto":true,"modoNoturno":false,"timestamp":12345683}
```

### 3. Enviar Comandos

#### Ativar Modo Automático

```powershell
mosquitto_pub -h localhost -t "semaforo/comandos" -m "auto"
```

#### Ativar Modo Normal

```powershell
mosquitto_pub -h localhost -t "semaforo/comandos" -m "normal"
```

#### Ativar Modo Noturno

```powershell
mosquitto_pub -h localhost -t "semaforo/comandos" -m "noturno"
```

### 4. Monitorar Comandos Recebidos

No Serial Monitor do ESP32, você verá:

```
[MQTT] Mensagem recebida no topico: semaforo/comandos
[MQTT] Conteudo: auto
[MQTT] Comando executado: Modo Automático
```

## ⚙️ Funcionamento do Sistema

### Modos de Operação

#### 1. Modo Automático (Padrão)

- **Ativação:** Automática no boot ou via interface web/MQTT
- **Funcionamento:**
  - Lê o valor do LDR continuamente
  - **Entra em modo NOTURNO** quando LDR < 1800
  - **Sai do modo NOTURNO** quando LDR > 2200
  - Usa histerese para evitar oscilações

#### 2. Modo Normal

- **Ativação:** Manual via interface web ou MQTT
- **Funcionamento:**
  - Ignora o sensor LDR
  - Executa ciclo completo do semáforo:
    - S1 Verde (3s) → S1 Amarelo (1.5s) → S2 Verde (3s) → S2 Amarelo (1.5s) → Repete

#### 3. Modo Noturno

- **Ativação:** Automática (quando escuro) ou manual
- **Funcionamento:**
  - Ambos os semáforos piscam amarelo simultaneamente
  - Intervalo: 500ms (ligado/desligado)

### Histerese do LDR

O sistema usa histerese para evitar oscilações frequentes:

- **Limite para entrar no modo NOTURNO:** LDR < 1800
- **Limite para sair do modo NOTURNO:** LDR > 2200
- **Zona morta:** Entre 1800 e 2200 (mantém o estado atual)

**Faixas esperadas:**
- **Noturno:** 0-2000
- **Diurno:** 2000-5000

### Ciclo Normal do Semáforo

```
Estado 0: S1 Verde | S2 Vermelho    (3 segundos)
Estado 1: S1 Amarelo | S2 Vermelho   (1.5 segundos)
Estado 2: S1 Vermelho | S2 Verde      (3 segundos)
Estado 3: S1 Vermelho | S2 Amarelo   (1.5 segundos)
→ Volta ao Estado 0
```

## 📊 Tópicos MQTT

### Publicação (ESP32 → Broker)

#### `semaforo/telemetria`

Publica dados a cada 5 segundos:

```json
{
  "luminosidade": 1450,
  "modoAuto": true,
  "modoNoturno": false,
  "timestamp": 12345678
}
```

### Subscrição (Broker → ESP32)

#### `semaforo/comandos`

Recebe comandos de controle:

- `"auto"` ou `"AUTO"` → Ativa modo automático
- `"normal"` ou `"NORMAL"` → Ativa modo normal
- `"noturno"` ou `"NOTURNO"` → Ativa modo noturno

## 🐛 Troubleshooting

### Problema: Interface web não carrega

**Soluções:**
1. Verifique se está conectado à rede Wi-Fi do ESP32
2. Confirme o IP no Serial Monitor (pode não ser 192.168.4.1)
3. Tente acessar pelo IP exibido no Serial Monitor
4. Verifique se o firewall não está bloqueando

### Problema: MQTT não conecta

**Soluções:**
1. Verifique se o Mosquitto está rodando: `mosquitto_sub -h localhost -t "test"`
2. Confirme que o PC está na mesma rede Wi-Fi do ESP32
3. Verifique o IP do broker no código (deve ser o IPv4 do PC)
4. Teste a conexão: `ping [IP_DO_PC]` do dispositivo conectado à rede do ESP32
5. Verifique o firewall (porta 1883 deve estar aberta)

### Problema: Semáforo sempre em modo noturno

**Soluções:**
1. Verifique a leitura do LDR no Serial Monitor
2. Ajuste os limites de histerese se necessário (linhas 107-108)
3. Verifique o circuito do LDR (divisor de tensão correto)
4. Teste cobrindo/descobrindo o LDR para ver mudanças

### Problema: LEDs não acendem

**Soluções:**
1. Verifique as conexões (ânodo/cátodo corretos)
2. Confirme que os resistores estão em série
3. Teste cada LED individualmente
4. Verifique se os pinos estão corretos no código

## 📁 Estrutura do Projeto

```
Ponderada04 - Semaforo Inteligente/
├── Ponderada04 - Semaforo Inteligente.ino  # Código principal
├── README.md                                 # Este arquivo
└── (outros arquivos de documentação)
```

## 🎓 Conceitos Aprendidos

- ✅ Programação Orientada a Objetos (POO) em Arduino
- ✅ Comunicação Wi-Fi com ESP32 (Access Point)
- ✅ Criação de servidor web embarcado
- ✅ Integração MQTT para IoT
- ✅ Sensores analógicos (LDR com ADC)
- ✅ Máquina de estados não bloqueante
- ✅ Histerese para controle de sistemas
- ✅ Interface web responsiva
- ✅ Comunicação bidirecional MQTT

## 📚 Referências

- [Documentação ESP32](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Biblioteca PubSubClient](https://github.com/knolleary/pubsubclient)
- [Mosquitto MQTT](https://mosquitto.org/)
- [Arduino CLI](https://arduino.github.io/arduino-cli/)
- [WebServer ESP32](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer)

## 🔄 Próximos Passos (Melhorias Futuras)

- [ ] Adicionar sensor de presença de veículos
- [ ] Implementar sincronização entre múltiplos semáforos via MQTT
- [ ] Adicionar histórico de dados (banco de dados)
- [ ] Criar dashboard web externo consumindo MQTT
- [ ] Implementar autenticação na interface web
- [ ] Adicionar notificações por email/SMS
- [ ] Integrar com sistemas de tráfego inteligente

## 📝 Notas Importantes

- O sistema continua funcionando mesmo se o broker MQTT não estiver disponível
- A interface web funciona independentemente do MQTT
- Os valores de histerese podem ser ajustados conforme o ambiente
- O Access Point do ESP32 permite até 4 conexões simultâneas

## 👤 Autor

Desenvolvido como parte do Módulo 4 de IOT do Instituto de Tecnologia e Liderança.

## 📄 Licença

Este projeto é de código aberto e está disponível para fins educacionais.

---

**Última atualização:** 2025

