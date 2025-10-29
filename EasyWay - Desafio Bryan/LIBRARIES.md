# EasyWay - Configuração de Bibliotecas Arduino

## 📚 Bibliotecas Necessárias

### Instalação via Arduino IDE Library Manager:

1. **ArduinoJson** (versão 6.21.3+)
   - Caminho: `C:/Users/Inteli/Documents/Arduino/libraries/ArduinoJson`
   - Função: Parsing e criação de JSON
   - Uso: Comunicação entre dispositivos

2. **CRC32** (versão 1.0.0+)
   - Caminho: `C:/Users/Inteli/Documents/Arduino/libraries/CRC32`
   - Função: Validação de integridade de dados
   - Uso: Checksum para segurança

### Bibliotecas Integradas ESP32:

3. **WiFi** (incluída no ESP32 Core)
   - Função: Comunicação Wi-Fi
   - Uso: Conexão de rede e comunicação UDP

4. **WiFiUdp** (incluída no ESP32 Core)
   - Função: Comunicação UDP
   - Uso: Troca de mensagens entre dispositivos

5. **WebServer** (incluída no ESP32 Core)
   - Função: Servidor HTTP
   - Uso: Interface web local

6. **SPIFFS** (incluída no ESP32 Core)
   - Função: Sistema de arquivos
   - Uso: Armazenamento de arquivos web

## 🔧 Configuração do VS Code

### Arquivo `.vscode/c_cpp_properties.json` atualizado:

```json
{
    "configurations": [
        {
            "name": "ESP32-WROOM-DA",
            "includePath": [
                "${workspaceFolder}/**",
                "C:/Users/Inteli/AppData/Local/Arduino15/packages/esp32/hardware/esp32/3.3.2/cores/esp32",
                "C:/Users/Inteli/AppData/Local/Arduino15/packages/esp32/hardware/esp32/3.3.2/variants/esp32",
                "C:/Users/Inteli/AppData/Local/Arduino15/packages/esp32/hardware/esp32/3.3.2/libraries/**",
                "C:/Users/Inteli/AppData/Local/Arduino15/packages/esp32/tools/**",
                "C:/Users/Inteli/AppData/Local/Arduino15/packages/esp32/hardware/esp32/3.3.2/**",
                "C:/Users/Inteli/Documents/Arduino/libraries/**",
                "C:/Users/Inteli/Documents/Arduino/libraries/ArduinoJson",
                "C:/Users/Inteli/Documents/Arduino/libraries/CRC32"
            ],
            "defines": [
                "ESP32",
                "ARDUINO_ARCH_ESP32",
                "ARDUINOJSON_ENABLE_PROGMEM=0",
                "ARDUINOJSON_ENABLE_ARDUINO_STRING=0"
            ],
            "compilerPath": "C:/Users/Inteli/AppData/Local/Arduino15/packages/esp32/tools/xtensa-esp-elf-gdb/16.3_20250913/bin/xtensa-esp32-elf-gdb.exe",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "gcc-x64"
        }
    ],
    "version": 4
}
```

## 📋 Checklist de Instalação

### ✅ Verificar Bibliotecas Instaladas:

1. **ArduinoJson**:
   - Abrir Arduino IDE
   - Tools → Manage Libraries
   - Procurar "ArduinoJson"
   - Verificar versão 6.21.3+
   - Instalar se necessário

2. **CRC32**:
   - Tools → Manage Libraries
   - Procurar "CRC32"
   - Verificar versão 1.0.0+
   - Instalar se necessário

3. **ESP32 Board Package**:
   - Tools → Board → Boards Manager
   - Procurar "ESP32"
   - Verificar versão 3.3.2+
   - Instalar se necessário

### ✅ Verificar Caminhos:

1. **ArduinoJson**: `C:/Users/Inteli/Documents/Arduino/libraries/ArduinoJson`
2. **CRC32**: `C:/Users/Inteli/Documents/Arduino/libraries/CRC32`
3. **ESP32 Core**: `C:/Users/Inteli/AppData/Local/Arduino15/packages/esp32/`

## 🚨 Solução de Problemas

### Erro: "ArduinoJson.h: No such file or directory"

**Soluções:**
1. Verificar se ArduinoJson está instalado
2. Verificar caminho da biblioteca
3. Reiniciar Arduino IDE
4. Verificar versão compatível

### Erro: "CRC32.h: No such file or directory"

**Soluções:**
1. Instalar biblioteca CRC32 via Library Manager
2. Verificar caminho da biblioteca
3. Reiniciar Arduino IDE

### Erro: "WiFi.h: No such file or directory"

**Soluções:**
1. Instalar ESP32 Board Package
2. Selecionar placa ESP32 Dev Module
3. Verificar versão do ESP32 Core

### Erro de Compilação no VS Code:

**Soluções:**
1. Atualizar arquivo `c_cpp_properties.json`
2. Reiniciar VS Code
3. Verificar IntelliSense
4. Limpar cache do VS Code

## 📊 Verificação de Instalação

### Teste Rápido:

```cpp
// Arquivo de teste para verificar bibliotecas
#include <ArduinoJson.h>
#include <CRC32.h>
#include <WiFi.h>
#include <WiFiUdp.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Bibliotecas carregadas com sucesso!");
}

void loop() {
  // Teste básico
}
```

### Logs Esperados:

```
Bibliotecas carregadas com sucesso!
```

## 🎯 Próximos Passos

1. **Verificar instalação** das bibliotecas
2. **Compilar projeto** EasyWay Server
3. **Compilar projeto** EasyWay Client
4. **Testar comunicação** entre dispositivos
5. **Verificar interface web** funcionando

## 📞 Suporte

### Informações de Debug:

- **Arduino IDE**: Verificar Library Manager
- **VS Code**: Verificar IntelliSense
- **Serial Monitor**: Logs de inicialização
- **Compilação**: Mensagens de erro detalhadas

### Logs Importantes:

**Sucesso:**
```
Bibliotecas carregadas com sucesso!
EasyWay Server - Iniciando sistema...
Comunicação WiFi inicializada com sucesso
```

**Erro:**
```
ArduinoJson.h: No such file or directory
CRC32.h: No such file or directory
WiFi.h: No such file or directory
```
