# 🌐 Configuração de Rede WiFi - EasyWay

## 📍 **Onde Alterar a Rede WiFi**

### **Servidor (EasyWay_Server_CLI)**

**Arquivo**: `EasyWay/EasyWay_Server_CLI/EasyWay_Server_CLI.ino`
**Linhas 19-20**:
```cpp
const char* WIFI_SSID = "MinhaRedeIoT";           // Nome da sua rede
const char* WIFI_PASSWORD = "minhasenha123";      // Senha da sua rede
```

### **Cliente (EasyWay_Client_CLI)**

**Arquivo**: `EasyWay/EasyWay_Client_CLI/EasyWay_Client_CLI.ino`
**Linhas 19-20**:
```cpp
const char* WIFI_SSID = "MinhaRedeIoT";           // Nome da rede do servidor
const char* WIFI_PASSWORD = "minhasenha123";      // Senha da rede do servidor
```

## 🔧 **Como Funciona o Sistema**

### **Modo Access Point (AP)**
- **Servidor**: Cria sua própria rede WiFi
- **Cliente**: Conecta-se à rede criada pelo servidor
- **IP do Servidor**: 192.168.4.1 (fixo)
- **IP dos Clientes**: 192.168.4.x (automático)

### **Vantagens do Modo AP**:
- ✅ Não depende de rede externa
- ✅ Funciona em qualquer lugar
- ✅ Controle total da rede
- ✅ Sem interferência externa

## ⚙️ **Configurações Atuais**

### **Rede Padrão**:
- **SSID**: `MinhaRedeIoT`
- **Senha**: `minhasenha123`
- **IP Servidor**: `192.168.4.1`
- **Porta**: `8080`

### **Para Alterar**:

1. **Escolha um nome para sua rede**:
   ```cpp
   const char* WIFI_SSID = "CasaInteligente";
   ```

2. **Escolha uma senha segura**:
   ```cpp
   const char* WIFI_PASSWORD = "MinhaSenhaSegura123";
   ```

3. **Altere em AMBOS os arquivos**:
   - Servidor: `EasyWay_Server_CLI.ino`
   - Cliente: `EasyWay_Client_CLI.ino`

## 🔄 **Processo de Alteração**

### **Passo 1: Alterar Servidor**
```cpp
// Em EasyWay_Server_CLI/EasyWay_Server_CLI.ino
const char* WIFI_SSID = "SuaRedePersonalizada";
const char* WIFI_PASSWORD = "SuaSenhaPersonalizada";
```

### **Passo 2: Alterar Cliente**
```cpp
// Em EasyWay_Client_CLI/EasyWay_Client_CLI.ino
const char* WIFI_SSID = "SuaRedePersonalizada";        // MESMO nome
const char* WIFI_PASSWORD = "SuaSenhaPersonalizada";   // MESMA senha
```

### **Passo 3: Compilar e Upload**
```bash
# Servidor
arduino-cli compile --fqbn esp32:esp32:esp32da .
arduino-cli upload --fqbn esp32:esp32:esp32da --port COM3 .

# Cliente
arduino-cli compile --fqbn esp32:esp32:esp32da .
arduino-cli upload --fqbn esp32:esp32:esp32da --port COM4 .
```

## 📱 **Como Conectar**

### **1. Servidor Inicia**
- Cria rede WiFi com nome escolhido
- IP: 192.168.4.1
- Interface web: http://192.168.4.1

### **2. Cliente Conecta**
- Procura pela rede do servidor
- Conecta automaticamente
- Recebe IP: 192.168.4.2, 192.168.4.3, etc.

### **3. Dispositivos Externos**
- Celular/PC pode conectar à rede
- Acessar interface web: http://192.168.4.1
- Controlar dispositivos IoT

## 🔍 **Verificação**

### **Serial Monitor do Servidor**:
```
Access Point criado: SuaRedePersonalizada
SSID do Access Point: SuaRedePersonalizada
IP do Access Point: 192.168.4.1
Número de clientes conectados: 1
```

### **Serial Monitor do Cliente**:
```
Conectando à rede WiFi: SuaRedePersonalizada
Conectado à rede WiFi!
IP Address: 192.168.4.2
```

## 🛠️ **Solução de Problemas**

### **Cliente não conecta**:
1. Verificar se SSID e senha são idênticos
2. Verificar se servidor está ativo
3. Verificar logs no Serial Monitor

### **Rede não aparece**:
1. Verificar se servidor está rodando
2. Verificar configuração do Access Point
3. Reiniciar servidor

### **IP não funciona**:
1. Verificar se IP é 192.168.4.1
2. Verificar se porta é 8080
3. Verificar firewall/antivírus

## 📋 **Exemplos de Configuração**

### **Casa Inteligente**:
```cpp
const char* WIFI_SSID = "CasaInteligente";
const char* WIFI_PASSWORD = "Casa2024!";
```

### **Escritório IoT**:
```cpp
const char* WIFI_SSID = "EscritorioIoT";
const char* WIFI_PASSWORD = "Office123#";
```

### **Laboratório**:
```cpp
const char* WIFI_SSID = "LabInteli";
const char* WIFI_PASSWORD = "Lab2024@";
```

## ⚠️ **Importante**

- **SSID e senha devem ser IDÊNTICOS** em servidor e cliente
- **Senha deve ter pelo menos 8 caracteres**
- **Evite caracteres especiais** que podem causar problemas
- **Teste sempre** após alterar configurações

---

**Agora você pode personalizar completamente a rede WiFi do seu sistema EasyWay!** 🌐

