# Código de Debug do Sensor PIR

## 📋 Descrição

Código simples para testar e configurar o sensor PIR. Mostra os valores lidos no Serial Monitor e acende o LED verde quando detecta movimento.

## 🚀 Como Usar

### 1. Abrir o Código

Abra a pasta `DEBUG_PIR` no Arduino IDE e abra o arquivo `DEBUG_PIR.ino`.

### 2. Configurar a Placa

- **Placa:** ESP32 Wrover Module (ou similar)
- **Porta:** Selecione a porta COM correta
- **Baudrate:** 115200

### 3. Fazer Upload

Compile e faça upload do código para a ESP32.

### 4. Abrir Serial Monitor

- Abra o Serial Monitor (Ctrl+Shift+M)
- Configure para **115200 baud**
- Você verá:

```
========================================
  DEBUG DO SENSOR PIR
========================================

Estado inicial do PIR: LOW (Sem movimento)

Iniciando monitoramento...
Formato: [Tempo] PIR=LOW/HIGH | LED=ON/OFF | Detecções: X
----------------------------------------

[1s] PIR=LOW | LED=OFF | Detecções: 0
[2s] PIR=LOW | LED=OFF | Detecções: 0
[3s] >>> MOVIMENTO DETECTADO! <<<
[3s] PIR=HIGH | LED=ON | Detecções: 1 | Duração: 0s
[4s] PIR=HIGH | LED=ON | Detecções: 1 | Duração: 1s
[5s] Movimento cessou.
[5s] PIR=LOW | LED=OFF | Detecções: 1
```

## 📊 O Que o Código Mostra

- **Estado do PIR:** LOW (sem movimento) ou HIGH (movimento detectado)
- **Estado do LED:** ON (ligado) ou OFF (desligado)
- **Contador de Detecções:** Número total de vezes que movimento foi detectado
- **Duração:** Quando há movimento, mostra há quanto tempo está ativo

## 🔍 Diagnóstico

### PIR Sempre em LOW
- ✅ **Normal:** PIR está funcionando, apenas não há movimento
- ❌ **Problema:** Se o LED verde nunca acender mesmo com movimento, verifique:
  - Conexão do PIR no GPIO 12
  - Alimentação do PIR (VCC e GND)
  - Sensor PIR pode estar com defeito

### PIR Sempre em HIGH
- ❌ **Problema:** PIR pode estar:
  - Desconectado (pino flutuando)
  - Com defeito
  - Muito sensível (ajustar potenciômetro se tiver)
  - Recebendo interferência

### PIR Alternando Rapidamente
- ❌ **Problema:** Pode ser:
  - Ruído elétrico
  - PIR muito sensível
  - Interferência de outras fontes (luz, calor, etc.)

## ⚙️ Ajustes

### Se o PIR Estiver Muito Sensível

Alguns sensores PIR têm potenciômetros para ajustar:
- **Sensibilidade:** Ajuste para menos sensível
- **Tempo de retenção:** Ajuste para manter o sinal HIGH por mais tempo

### Se o PIR Não Estiver Detectando

1. Verifique as conexões:
   - **VCC** → 5V ou 3.3V
   - **GND** → GND
   - **OUT** → GPIO 12

2. Aguarde 30-60 segundos após ligar (PIRs precisam de tempo para estabilizar)

3. Verifique se há movimento real na área de detecção

## 🔄 Voltar ao Código Principal

Após configurar o PIR, volte a usar o código principal:
- `DroneBotWorkshopCode/DroneBotWorkshopCode.ino`

---

**Dica:** Deixe este código rodando por alguns minutos para observar o comportamento do PIR e identificar padrões de falsos positivos.


