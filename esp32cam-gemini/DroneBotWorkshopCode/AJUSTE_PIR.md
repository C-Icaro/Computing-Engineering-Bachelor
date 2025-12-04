# Ajuste do Sensor PIR - Redução de Falsos Positivos

## 🔧 Melhorias Implementadas

Foram implementadas várias melhorias no código para reduzir falsos positivos do sensor PIR:

### 0. **Pull-Down Habilitado (Correção Crítica)**
- **Problema:** Quando o PIR não está conectado, o pino fica flutuando e pode ler HIGH aleatoriamente
- **Solução:** Pull-down habilitado no pino do PIR para manter em LOW quando desconectado
- **Resultado:** Evita falsos positivos quando o sensor não está conectado ao circuito

### 1. **Detecção de PIR Desconectado**
- **Novo:** Verificação na inicialização para detectar se o PIR está conectado
- **Motivo:** Evita processar detecções quando o sensor não está no circuito
- **Comportamento:** Se o PIR não estiver conectado, o sistema ignora detecções e desabilita wake-up do deep sleep

### 2. **Debounce Aumentado**
- **Antes:** 100ms
- **Agora:** 500ms
- **Motivo:** PIRs precisam de mais tempo para estabilizar após detecção

### 3. **Período de Cooldown**
- **Novo:** 10 segundos entre detecções
- **Motivo:** Evita múltiplas detecções seguidas do mesmo movimento
- **Configurável:** `pirCooldownPeriod`

### 4. **Verificação de Estabilidade**
- **Novo:** Requer 5 leituras consecutivas HIGH para confirmar detecção
- **Motivo:** Filtra ruídos e oscilações rápidas
- **Configurável:** `pirStabilityReadings`

### 5. **Validação Após Deep Sleep**
- **Novo:** Após acordar do deep sleep, aguarda 2 segundos e verifica novamente
- **Motivo:** PIRs podem dar sinal falso imediatamente após acordar
- **Validação:** Verifica 10 vezes em 1 segundo se o PIR ainda está ativo

## ⚙️ Parâmetros Configuráveis

No código, você pode ajustar os seguintes parâmetros (linha ~475):

```cpp
const unsigned long pirDebounceDelay = 500;        // ms - Tempo de debounce
const unsigned long pirCooldownPeriod = 10000;     // ms - Período de cooldown (10 segundos)
const int pirStabilityReadings = 5;                // Número de leituras consecutivas necessárias
```

### Ajuste do Debounce Delay

- **Valores menores (200-300ms):** Mais sensível, pode ter mais falsos positivos
- **Valores maiores (500-1000ms):** Menos sensível, mais estável
- **Recomendado:** 500ms para a maioria dos casos

### Ajuste do Cooldown Period

- **Valores menores (5-8s):** Permite detecções mais frequentes
- **Valores maiores (15-30s):** Reduz significativamente falsos positivos
- **Recomendado:** 10 segundos para uso geral

### Ajuste do Stability Readings

- **Valores menores (3-4):** Mais rápido, mas menos estável
- **Valores maiores (6-10):** Mais estável, mas pode perder detecções rápidas
- **Recomendado:** 5 leituras para equilíbrio

## 🔍 Como Funciona

### Fluxo de Detecção Normal (Loop Principal)

1. **Leitura do PIR:** Lê o estado do pino PIR
2. **Verificação de Cooldown:** Se ainda está em período de cooldown, ignora
3. **Debounce:** Aguarda estabilização do sinal (500ms)
4. **Verificação de Estabilidade:** Requer 5 leituras consecutivas HIGH
5. **Confirmação:** Se todas as condições forem atendidas, processa a captura
6. **Cooldown:** Inicia período de 10 segundos antes de aceitar nova detecção

### Fluxo Após Deep Sleep

1. **Acorda pelo PIR:** Sistema acorda do deep sleep
2. **Delay de Estabilização:** Aguarda 2 segundos para o PIR estabilizar
3. **Validação:** Verifica 10 vezes em 1 segundo se o PIR ainda está ativo
4. **Confirmação:** Se confirmado, processa a captura
5. **Rejeição:** Se não confirmado, ignora como falso positivo

## 📊 Exemplo de Ajuste para Diferentes Cenários

### Cenário 1: Ambiente com Muito Ruído
```cpp
const unsigned long pirDebounceDelay = 1000;        // 1 segundo
const unsigned long pirCooldownPeriod = 20000;     // 20 segundos
const int pirStabilityReadings = 7;                // 7 leituras
```

### Cenário 2: Ambiente Estável, Detecções Rápidas Necessárias
```cpp
const unsigned long pirDebounceDelay = 300;        // 300ms
const unsigned long pirCooldownPeriod = 5000;      // 5 segundos
const int pirStabilityReadings = 3;                // 3 leituras
```

### Cenário 3: Uso Geral (Recomendado)
```cpp
const unsigned long pirDebounceDelay = 500;        // 500ms
const unsigned long pirCooldownPeriod = 10000;     // 10 segundos
const int pirStabilityReadings = 5;                // 5 leituras
```

## 🐛 Troubleshooting

### Ainda Tem Muitos Falsos Positivos?

1. **Aumente o `pirCooldownPeriod`:**
   - Tente 15-20 segundos
   - Isso reduzirá significativamente as detecções

2. **Aumente o `pirStabilityReadings`:**
   - Tente 7-10 leituras
   - Isso tornará a detecção mais rigorosa

3. **Aumente o `pirDebounceDelay`:**
   - Tente 1000ms (1 segundo)
   - Isso dará mais tempo para o PIR estabilizar

### Não Está Detectando Movimento Real?

1. **Reduza o `pirCooldownPeriod`:**
   - Tente 5-8 segundos
   - Isso permitirá detecções mais frequentes

2. **Reduza o `pirStabilityReadings`:**
   - Tente 3-4 leituras
   - Isso tornará a detecção mais sensível

3. **Reduza o `pirDebounceDelay`:**
   - Tente 300ms
   - Isso tornará a resposta mais rápida

### Verificar no Serial Monitor

O código agora imprime mensagens úteis:

```
PIR detectou movimento, aguardando confirmação (3/5)...
Movimento detectado pelo PIR (confirmado após estabilidade)!
PIR em cooldown (7s restantes)...
PIR acionou mas não confirmou após estabilização - provável falso positivo, ignorando...
```

Use essas mensagens para entender o comportamento do PIR e ajustar os parâmetros conforme necessário.

## ⚠️ Problema: PIR Desconectado

### Sintoma
O ESP32 detecta movimento mesmo quando o sensor PIR não está conectado ao circuito.

### Causa
O pino GPIO 12 (PIR_PIN) fica flutuando quando o sensor não está conectado, gerando leituras aleatórias HIGH/LOW.

### Solução Implementada
1. **Pull-down habilitado:** O pino agora tem pull-down interno habilitado, mantendo em LOW quando desconectado
2. **Verificação de conexão:** O código verifica se o PIR está conectado na inicialização
3. **Ignorar detecções:** Se o PIR não estiver conectado, todas as detecções são ignoradas
4. **Wake-up desabilitado:** O wake-up do deep sleep pelo PIR é desabilitado se o sensor não estiver conectado

### Verificação
Após compilar e fazer upload, o Serial Monitor mostrará:
- `"PIR inicializado (pull-down habilitado para evitar leituras flutuantes)"` - PIR conectado corretamente
- `"AVISO: PIR detectou sinal HIGH na inicialização..."` - Verificar conexão
- `"AVISO: Sistema acordou pelo PIR, mas sensor pode estar desconectado. Ignorando..."` - PIR desconectado

## 📝 Notas Importantes

1. **Conexão do PIR:**
   - Certifique-se de que o PIR está conectado ao GPIO 12
   - Verifique as conexões: VCC, GND e OUT (GPIO 12)
   - O pull-down interno agora mantém o pino em LOW quando desconectado

2. **Posicionamento do PIR:**
   - Evite posicionar o PIR diretamente sob luz solar
   - Evite correntes de ar (ventiladores, ar condicionado)
   - Evite fontes de calor (lâmpadas, aquecedores)

2. **Ajuste Físico:**
   - Alguns PIRs têm potenciômetros para ajustar sensibilidade
   - Consulte a documentação do seu modelo específico

3. **Teste e Ajuste:**
   - Cada ambiente é diferente
   - Ajuste os parâmetros baseado no comportamento observado
   - Use o Serial Monitor para monitorar o comportamento

## 🔄 Reversão das Mudanças

Se precisar reverter para o comportamento anterior (mais sensível, mas com mais falsos positivos):

```cpp
const unsigned long pirDebounceDelay = 100;        // ms
const unsigned long pirCooldownPeriod = 0;         // Sem cooldown
const int pirStabilityReadings = 1;                // Sem verificação de estabilidade
```

**⚠️ Aviso:** Isso aumentará significativamente os falsos positivos.

---

**Última atualização:** Dezembro 2024

