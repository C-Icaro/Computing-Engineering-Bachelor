Ponderada de computação proposta pelo professor Bryan com objetivo de treinar os fundamentos de POO e ponteiros.

## Semáforo no ESP-WROOM-32U (ESP32)

Este projeto implementa um semáforo simples usando ESP32, Programação Orientada a Objetos (POO) e ponteiros. O ciclo é não bloqueante (sem `delay`), usando `millis()` para temporização.

### Mapeamento de pinos
- Led verde: Pino 27
- Led amarelo: Pino 26
- Led vermelho: Pino 25

### Requisitos
- Arduino IDE (ou PlatformIO)
- Placa ESP32 instalada na IDE
- Três LEDs (verde, amarelo e vermelho) e resistores adequados (220–330 Ω)

### Ligações elétricas
- Anodo de cada LED no respectivo pino (`27`, `26`, `25`)
- Catodo de cada LED ao GND por meio de um resistor em série

### Código-fonte
O arquivo principal é `Ponderada03 - Semaforo.ino`. Ele define:
- Classe `Led`: encapsula um pino digital com métodos `begin()`, `on()` e `off()`.
- Enum `LightState`: representa os estados `Green`, `Yellow`, `Red`.
- Classe `TrafficLight`: máquina de estados não bloqueante que recebe ponteiros para `Led` e tempos de cada fase.

Os objetos `Led` e `TrafficLight` são criados dinamicamente com `new` (uso de ponteiros) e atualizados no `loop()` via `update()`.

### Temporizações padrão
- Verde: 5000 ms
- Amarelo: 1000 ms
- Vermelho: 5000 ms

Você pode alterar esses tempos no `setup()` ao instanciar `TrafficLight`.

### Como compilar e carregar
1. Abra a pasta do projeto na Arduino IDE.
2. Selecione a placa: Ferramentas → Placa → ESP32 → ESP32 Dev Module (ou a variante correspondente ao seu ESP-WROOM-32U).
3. Selecione a porta correta (COM).
4. Carregue o sketch.

### Como funciona (POO e ponteiros)
- O `TrafficLight` recebe `Led*` (ponteiros) para verde, amarelo e vermelho.
- O método `update()` troca de estado quando o tempo da fase expira, medido por `millis()`.
- Não há `delay()`, permitindo que o loop permaneça responsivo.

### Ajustes
- Pinos: altere os números passados a `Led(27)`, `Led(26)`, `Led(25)` se necessário.
- Tempos: altere os valores (em ms) ao criar `TrafficLight` no `setup()`.

### Licença
Uso educacional.
