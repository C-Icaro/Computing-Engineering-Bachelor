# BME280 Test - Sensor de Temperatura, Umidade e Pressão

## 📋 Descrição

Este sketch funcional demonstra como usar o sensor **BME280** para ler temperatura, umidade, pressão atmosférica e calcular altitude aproximada. Este é o código que **funcionou** após identificar corretamente o sensor através do Sensor ID 0x60.

## 🎯 Objetivo

- Inicializar corretamente o BME280 no endereço 0x76
- Ler temperatura, umidade e pressão continuamente
- Calcular altitude aproximada baseada na pressão
- Exibir leituras formatadas no Serial Monitor

## ✅ Este Código Funcionou!

Este exemplo foi testado e validado com:
- **Sensor ID detectado**: 0x60 (BME280)
- **Endereço I2C**: 0x76
- **Placa**: ESP32
- **Resultado**: Leituras bem-sucedidas de todos os parâmetros

## 🔧 Hardware

### Sensor BME280
- **Fabricante**: Bosch
- **Medições**: Temperatura, Umidade e Pressão
- **Endereços I2C**: 0x76 ou 0x77 (configurável por jumper)
- **Sensor ID**: 0x60
- **Alimentação**: 3.3V recomendado (ou 5V em alguns breakouts)

### ⚠️ Diferença Crítica: BME280 vs BMP280

| Característica | **BME280** (Este projeto) | BMP280 |
|----------------|---------------------------|---------|
| **Sensor ID** | **0x60** | 0x56-0x58 |
| **Temperatura** | ✅ Sim | ✅ Sim |
| **Pressão** | ✅ Sim | ✅ Sim |
| **Umidade** | ✅ **SIM** | ❌ **NÃO** |
| **Biblioteca** | `Adafruit_BME280` | `Adafruit_BMP280` |
| **Include** | `<Adafruit_BME280.h>` | `<Adafruit_BMP280.h>` |
| **Objeto** | `Adafruit_BME280 bme` | `Adafruit_BMP280 bmp` |

> **💡 Importante**: Se você tentou usar código BMP280 e recebeu "SensorID: 0x60", você tem um BME280! Use este código.

### Conexões I2C (Testadas no ESP32)

```
BME280 VCC  → 3.3V
BME280 GND  → GND
BME280 SCL  → GPIO 22
BME280 SDA  → GPIO 21
```

#### Outras Placas

**Arduino Uno/Nano:**
```
SCL → A5
SDA → A4
```

**Arduino Mega:**
```
SCL → Pin 21
SDA → Pin 20
```

## 📚 Bibliotecas Necessárias

### ⚠️ IMPORTANTE: Use a Biblioteca Correta!

```
✅ Adafruit BME280 Library  (CORRETO - com "E")
❌ Adafruit BMP280 Library  (ERRADO - sem "E")
```

Dependências (instaladas automaticamente):
- Adafruit Unified Sensor

### Instalação via Arduino IDE
1. **Sketch → Include Library → Manage Libraries**
2. Busque: **"Adafruit BME280"** (com E!)
3. Clique em Install
4. Confirme instalação da dependência "Adafruit Unified Sensor"

### Instalação via Arduino CLI
```powershell
arduino-cli lib install "Adafruit BME280 Library"
arduino-cli lib install "Adafruit Unified Sensor"
```

## 💻 Código - Detalhamento

### 1. Includes Corretos
```cpp
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>  // Note: BME, não BMP!
```

### 2. Definição do Endereço
```cpp
#define BME280_ADDRESS 0x76  // Endereço detectado no I2C scan
```

### 3. Criação do Objeto
```cpp
Adafruit_BME280 bme;  // Note: "bme", não "bmp"
```

### 4. Inicialização
```cpp
if (!bme.begin(BME280_ADDRESS)) {
  Serial.println(F("ERROR: Could not find BME280 sensor at 0x76!"));
  while (1) delay(100);
}
```

### 5. Configuração de Amostragem
```cpp
bme.setSampling(
  Adafruit_BME280::MODE_NORMAL,
  Adafruit_BME280::SAMPLING_X2,   // Temperatura
  Adafruit_BME280::SAMPLING_X16,  // Pressão
  Adafruit_BME280::SAMPLING_X1,   // Umidade (único para BME280)
  Adafruit_BME280::FILTER_X16,
  Adafruit_BME280::STANDBY_MS_500
);
```

### 6. Leituras
```cpp
float temperature = bme.readTemperature();        // °C
float humidity = bme.readHumidity();              // %
float pressure = bme.readPressure() / 100.0F;     // hPa
float altitude = bme.readAltitude(SEALEVEL_HPA);  // metros
```

## 🚀 Como Usar

### 1. Instalar Bibliotecas
Certifique-se de instalar **Adafruit BME280** (não BMP280!)

### 2. Upload do Código
```powershell
# Compilar (ajuste o FQBN para sua placa)
arduino-cli compile --fqbn esp32:esp32:esp32da Tests/BME280_Test

# Upload (ajuste a porta)
arduino-cli upload -p COM11 --fqbn esp32:esp32:esp32da Tests/BME280_Test
```

Ou pelo Arduino IDE:
1. Abra `BME280_Test.ino`
2. Selecione a placa e porta
3. Clique em Upload

### 3. Abrir Serial Monitor
- **Baud rate**: 115200 (importante!)
- `arduino-cli monitor -p COM11 -b 115200`
- Ou pelo IDE: Tools → Serial Monitor

## 📊 Saída Esperada

```
=== BME280 Test (Sensor ID: 0x60) ===
Initializing BME280 at 0x76
BME280 initialized successfully!
Sensor ID: 0x60

Starting readings...

--- BME280 Readings ---
Temperature: 23.45 °C
Humidity: 45.67 %
Pressure: 1013.25 hPa
Approx. Altitude: 123.45 m

--- BME280 Readings ---
Temperature: 23.46 °C
Humidity: 45.65 %
Pressure: 1013.24 hPa
Approx. Altitude: 123.50 m
```

## 📈 Interpretação das Leituras

### Temperatura
- **Range**: -40°C a +85°C
- **Precisão**: ±1°C (0-65°C)
- **Uso**: Monitoramento ambiental, compensação de outros sensores

### Umidade (Exclusivo do BME280!)
- **Range**: 0-100%
- **Precisão**: ±3% (20-80%)
- **Uso**: Conforto térmico, prevenção de condensação, agricultura

### Pressão Atmosférica
- **Range**: 300-1100 hPa
- **Precisão**: ±1 hPa
- **Uso**: Previsão do tempo, altimetria
- **Nota**: 1 hPa = 1 mbar

### Altitude Aproximada
- Calculada pela fórmula barométrica
- Depende da pressão ao nível do mar (`SEALEVEL_HPA`)
- **Ajuste**: Busque a pressão atual da sua cidade em sites de meteorologia
- Exemplo: Se em São Paulo a pressão ao nível do mar for 1015 hPa, altere:
  ```cpp
  const float SEALEVEL_HPA = 1015.0;  // Ajustar conforme local
  ```

## 🔍 Troubleshooting

### Problema: "Could not find BME280 sensor"
**Soluções**:
1. Verifique se instalou biblioteca **BME280** (não BMP280)
2. Confirme endereço I2C (rode I2C scan primeiro)
3. Verifique conexões SDA/SCL
4. Confirme alimentação (3.3V recomendado)

### Problema: Leituras de Umidade Sempre 0% ou 100%
**Causas**:
- Configuração de oversampling incorreta
- Sensor ainda não estabilizado

**Soluções**:
- Aguarde ~30 segundos após inicialização
- Verifique configuração `SAMPLING_X1` para umidade

### Problema: Altitude Incorreta
**Causa**: Valor `SEALEVEL_HPA` inadequado

**Solução**:
1. Acesse [tempo.com](https://www.tempo.com) ou similar
2. Busque pressão ao nível do mar da sua região
3. Atualize `SEALEVEL_HPA` no código

### Problema: Pressão Muito Baixa ou Alta
**Verificações**:
- Valores normais: 980-1030 hPa ao nível do mar
- Se < 900 ou > 1100: problema de leitura
- Verifique se `/ 100.0F` está presente (converte Pa → hPa)

## 🎓 O Que Aprendi Neste Projeto

### 1. Identificação Correta do Sensor
- ✅ Sempre fazer I2C scan primeiro
- ✅ Verificar Sensor ID para confirmar modelo
- ✅ 0x60 = BME280, 0x56-0x58 = BMP280

### 2. Importância da Biblioteca Correta
- ❌ Código BMP280 **não funciona** com sensor BME280
- ✅ Biblioteca deve corresponder exatamente ao chip
- ✅ Includes, objetos e métodos são diferentes

### 3. Diagnóstico Sistemático
1. I2C Scan → Detecta endereço
2. Leitura de Sensor ID → Identifica modelo
3. Biblioteca correta → Funciona!

### 4. Diferenças de Hardware
- BME280 tem sensor de umidade adicional
- Requer oversampling específico para umidade
- Sensor ID é único e confiável para identificação

## 🆚 Comparação: Código BMP280 vs BME280

### Includes
```cpp
// BMP280 (ERRADO para nosso sensor)
#include <Adafruit_BMP280.h>

// BME280 (CORRETO)
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>  // Adicional necessário
```

### Objeto
```cpp
// BMP280
Adafruit_BMP280 bmp;

// BME280
Adafruit_BME280 bme;
```

### Leituras
```cpp
// BMP280 - Apenas 2 sensores
float temp = bmp.readTemperature();
float press = bmp.readPressure();

// BME280 - 3 sensores
float temp = bme.readTemperature();
float hum = bme.readHumidity();      // EXCLUSIVO!
float press = bme.readPressure();
```

### Configuração
```cpp
// BMP280 - Sem umidade
bmp.setSampling(...,
  SAMPLING_X2,   // Temperatura
  SAMPLING_X16,  // Pressão
  ...);

// BME280 - Inclui umidade
bme.setSampling(...,
  SAMPLING_X2,   // Temperatura
  SAMPLING_X16,  // Pressão
  SAMPLING_X1,   // Umidade (novo parâmetro!)
  ...);
```

## 📖 Referências

- [Datasheet BME280](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf)
- [Adafruit BME280 Guide](https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout)
- [Diferenças BMP vs BME](https://www.bosch-sensortec.com/products/environmental-sensors/)

## 💡 Dicas Importantes

1. **Sensor ID é definitivo**: 0x60 sempre indica BME280
2. **Biblioteca errada não funciona**: Mesmo que compile, não inicializa
3. **I2C Scan é essencial**: Sempre começar com diagnóstico
4. **Baud rate correto**: 115200 para ver todas as mensagens
5. **Alimentação estável**: 3.3V é mais seguro que 5V
6. **Cabos curtos**: I2C não funciona bem em distâncias > 30cm
7. **Pull-ups**: Breakouts geralmente já tem, não adicionar duplicados

## 🎉 Status

✅ **CÓDIGO FUNCIONAL E TESTADO**

Este código foi validado e está produzindo leituras corretas de:
- Temperatura
- Umidade
- Pressão
- Altitude

## 👤 Autor

Desenvolvido como parte do Módulo 4 - IOT

Tempo investido: Muito! Mas valeu a pena aprender a diferença entre BMP280 e BME280.

## 📄 Licença

Este projeto é de código aberto e está disponível para fins educacionais.
