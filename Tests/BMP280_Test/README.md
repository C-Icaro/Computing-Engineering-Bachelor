# BMP280 Test - Diagnóstico e Varredura I2C

## 📋 Descrição

Este sketch é uma ferramenta de diagnóstico para testar sensores BMP280 via I2C. Ele realiza uma varredura completa do barramento I2C, tenta inicializar o sensor em múltiplos endereços e fornece informações detalhadas para troubleshooting.

## 🎯 Objetivo

- Detectar dispositivos I2C conectados
- Tentar inicializar BMP280 nos endereços comuns (0x76 e 0x77)
- Identificar o tipo de sensor através do sensor ID
- Fornecer diagnóstico detalhado em caso de falha
- Ler temperatura, pressão e altitude (quando bem-sucedido)

## 🔧 Hardware

### Sensor BMP280
- **Fabricante**: Bosch
- **Medições**: Temperatura e Pressão (SEM umidade)
- **Endereços I2C**: 0x76 ou 0x77 (configurável por jumper)
- **Sensor ID**: 0x56, 0x57 ou 0x58
- **Alimentação**: 3.3V ou 5V (dependendo do módulo breakout)

### Conexões I2C

#### Arduino Uno/Nano
```
BMP280 VCC  → 3.3V ou 5V
BMP280 GND  → GND
BMP280 SCL  → A5
BMP280 SDA  → A4
```

#### ESP32
```
BMP280 VCC  → 3.3V
BMP280 GND  → GND
BMP280 SCL  → GPIO 22
BMP280 SDA  → GPIO 21
```

#### Arduino Mega
```
BMP280 VCC  → 3.3V ou 5V
BMP280 GND  → GND
BMP280 SCL  → Pin 21
BMP280 SDA  → Pin 20
```

## 📚 Biblioteca Necessária

```
Adafruit BMP280 Library
```

### Instalação via Arduino IDE
1. Sketch → Include Library → Manage Libraries
2. Busque: "Adafruit BMP280"
3. Clique em Install

### Instalação via Arduino CLI
```powershell
arduino-cli lib install "Adafruit BMP280 Library"
```

## 💻 Funcionalidades do Código

### 1. Varredura I2C (`scanI2C()`)
```cpp
void scanI2C() {
  // Varre endereços de 0x01 a 0x7F
  // Identifica dispositivos respondendo no barramento
}
```
- Lista todos os dispositivos I2C conectados
- Útil para verificar se o sensor está sendo detectado
- Mostra endereços em formato hexadecimal

### 2. Inicialização Multi-endereço (`initBMP280()`)
```cpp
bool initBMP280() {
  const uint8_t addrs[] = {0x76, 0x77};
  // Tenta ambos os endereços comuns
}
```
- Tenta 0x76 primeiro (endereço padrão mais comum)
- Se falhar, tenta 0x77 (endereço alternativo)
- Retorna `true` se inicialização for bem-sucedida

### 3. Diagnóstico Avançado
- Lê e exibe o **Sensor ID** do chip
- Fornece interpretação do ID:
  - `0x56-0x58` → BMP280 (correto)
  - `0x60` → **BME280** (sensor diferente, precisa biblioteca BME280)
  - `0xFF` → Erro de endereço ou conexão
  - `0x00` → Sensor não respondendo

### 4. Configuração de Amostragem
```cpp
bmp.setSampling(
  Adafruit_BMP280::MODE_NORMAL,      // Modo contínuo
  Adafruit_BMP280::SAMPLING_X2,      // Oversampling temperatura
  Adafruit_BMP280::SAMPLING_X16,     // Oversampling pressão
  Adafruit_BMP280::FILTER_X16,       // Filtro IIR
  Adafruit_BMP280::STANDBY_MS_500    // Intervalo entre leituras
);
```

## 🚀 Como Usar

### 1. Upload do Código
1. Abra `BMP280_Test.ino` no Arduino IDE
2. Selecione a placa correta em **Tools → Board**
3. Selecione a porta correta em **Tools → Port**
4. Clique em **Upload**

### 2. Abrir Serial Monitor
- Baud rate: **115200**
- Tools → Serial Monitor ou `Ctrl+Shift+M`

### 3. Interpretar Resultados

#### ✅ Sucesso
```
=== BMP280 Test ===

-- I2C Scan --
Found I2C device at 0x76
-- End Scan --

Trying BMP280 at 0x76
BMP280 initialized at 0x76
Detected sensorID: 0x58

Temperature: 23.45 °C
Pressure: 1013.25 hPa
Approx. altitude: 123.45 m
```

#### ⚠️ Sensor Errado (BME280 detectado)
```
-- I2C Scan --
Found I2C device at 0x76
-- End Scan --

Trying BMP280 at 0x76
Trying BMP280 at 0x77
Could not initialize BMP280 at 0x76 or 0x77.
Raw sensorID: 0x60
Hints: 0x60=BME280 (use BME library)
```

**Solução**: Use o sketch BME280_Test e instale a biblioteca `Adafruit BME280`

#### ❌ Sensor Não Detectado
```
-- I2C Scan --
No I2C devices found
-- End Scan --
```

**Soluções**:
1. Verifique as conexões SDA/SCL
2. Confirme alimentação (VCC e GND)
3. Verifique se há GND comum entre Arduino e sensor
4. Teste com outro sensor para descartar defeito

## 🔍 Troubleshooting

### Problema: "No I2C devices found"
**Causas**:
- Cabos soltos ou mal conectados
- Inversão de SDA/SCL
- Falta de alimentação no sensor
- GND não comum

**Solução**: Revisar todas as conexões físicas

### Problema: "SensorID: 0x60"
**Causa**: Você tem um BME280, não BMP280

**Solução**: Use o sketch `BME280_Test` e instale biblioteca BME280

### Problema: "SensorID: 0xFF"
**Causas**:
- Endereço I2C incorreto
- Sensor BMP180 (modelo antigo)

**Solução**: Confirme o modelo do sensor

### Problema: "SensorID: 0x00"
**Causas**:
- Sensor danificado
- Alimentação inadequada
- Nível lógico incompatível (3.3V vs 5V)

**Solução**: Verifique alimentação e teste com 3.3V

## 📊 Leituras do Sensor

### Temperatura
- Range: -40°C a +85°C
- Precisão: ±1°C
- Uso: Ambiente, correção de pressão

### Pressão
- Range: 300-1100 hPa
- Precisão: ±1 hPa
- Uso: Previsão do tempo, altimetria

### Altitude Aproximada
- Calculada a partir da pressão
- Requer pressão ao nível do mar (`SEALEVEL_HPA`)
- Ajuste 1013.25 para pressão local para maior precisão

## 🆚 BMP280 vs BME280

| Característica | BMP280 | BME280 |
|----------------|---------|---------|
| **Sensor ID** | 0x56-0x58 | 0x60 |
| **Temperatura** | ✅ | ✅ |
| **Pressão** | ✅ | ✅ |
| **Umidade** | ❌ | ✅ |
| **Biblioteca** | Adafruit_BMP280 | Adafruit_BME280 |
| **Preço** | Menor | Maior |

## 📖 Referências

- [Datasheet BMP280](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmp280-ds001.pdf)
- [Adafruit BMP280 Guide](https://learn.adafruit.com/adafruit-bmp280-barometric-pressure-plus-temperature-sensor-breakout)
- [I2C Protocol Basics](https://learn.sparkfun.com/tutorials/i2c)

## 💡 Dicas

1. **Alimentação**: Prefira 3.3V para maior compatibilidade
2. **Pull-ups**: Módulos breakout geralmente já incluem resistores de 4.7kΩ
3. **Cabos**: Use cabos curtos (< 30cm) para comunicação I2C estável
4. **Diagnóstico**: Sempre rode o I2C scan primeiro para confirmar detecção

## 👤 Autor

Desenvolvido como parte do Módulo 4 - IOT

## 📄 Licença

Este projeto é de código aberto e está disponível para fins educacionais.
