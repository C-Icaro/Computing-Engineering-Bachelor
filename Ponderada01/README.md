# Ponderada 01 - Blink LED Arduino

## 📋 Descrição

Este projeto implementa o programa básico "Blink" em Arduino, que consiste em fazer um LED piscar em intervalos regulares de 1 segundo. É um dos projetos mais fundamentais para aprender programação de microcontroladores e eletrônica básica.

## 🎯 Objetivo

Demonstrar o funcionamento básico de:
- Programação Arduino
- Controle de saídas digitais
- Temporização com a função `delay()`
- Uso do LED integrado e LED externo

## 🔧 Hardware Necessário

- 1x Placa Arduino (Uno, Mega, Nano, etc.)
- 1x LED (para versão com LED externo)
- 1x Resistor 220Ω ou 330Ω (para versão com LED externo)
- Jumpers para conexões
- Cabo USB para programação

## 📁 Estrutura do Projeto

```
Ponderada01/
├── blink.ino              # Código fonte do projeto
├── BlinkFísico.jpg        # Foto da implementação física
├── BlinkLedExterno.png    # Diagrama com LED externo
├── BlinkTinkercad.png     # Simulação no Tinkercad
└── README.md              # Este arquivo
```

## 💻 Código

O código principal está em `blink.ino` e realiza as seguintes operações:

```cpp
void setup() {
  // Configura o pino do LED como saída
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // Liga o LED
  delay(1000);                      // Aguarda 1 segundo
  digitalWrite(LED_BUILTIN, LOW);   // Desliga o LED
  delay(1000);                      // Aguarda 1 segundo
}
```

### Como Funciona

1. **setup()**: Configura o pino LED_BUILTIN (LED integrado na placa) como saída
2. **loop()**: Executa continuamente:
   - Liga o LED (HIGH)
   - Aguarda 1000ms (1 segundo)
   - Desliga o LED (LOW)
   - Aguarda 1000ms (1 segundo)

## 🚀 Como Usar

### 1. Instalação do Arduino IDE

Baixe e instale o [Arduino IDE](https://www.arduino.cc/en/software) ou use o [Arduino CLI](https://arduino.github.io/arduino-cli/).

### 2. Carregar o Código

1. Abra o arquivo `blink.ino` no Arduino IDE
2. Conecte sua placa Arduino ao computador via USB
3. Selecione a placa correta em: **Ferramentas > Placa**
4. Selecione a porta correta em: **Ferramentas > Porta**
5. Clique no botão **Upload** (seta para direita)

### 3. Observar o Resultado

Após o upload, o LED integrado da placa Arduino começará a piscar em intervalos de 1 segundo.

## 🔌 Versão com LED Externo

Para usar um LED externo ao invés do LED integrado:

### Conexões

1. LED ânodo (perna longa) → Resistor 220Ω → Pino digital (ex: pino 13)
2. LED cátodo (perna curta) → GND

### Modificação no Código

Substitua `LED_BUILTIN` pelo número do pino digital escolhido:

```cpp
#define LED_PIN 13

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
```

## 📸 Demonstrações

### Simulação no Tinkercad
<p align="center"><em>Figura 1: Simulação do circuito Blink no Tinkercad</em></p>

![Simulação no Tinkercad](BlinkTinkercad.png)

<p align="center"><strong>Fonte:</strong> Carlos Icaro, 2025</p>

### LED Externo - Diagrama de Conexão
<p align="center"><em>Figura 2: Esquema de conexão com LED externo</em></p>

![Diagrama LED Externo](BlinkLedExterno.png)

<p align="center"><strong>Fonte:</strong> Carlos Icaro, 2025</p>

### Implementação Física Blink Interno
<p align="center"><em>Figura 3: Montagem física do circuito com Arduino</em></p>

![Implementação Física](BlinkFísico.jpg)

<p align="center"><strong>Fonte:</strong> Carlos Icaro, 2025</p>

### 🎥 Vídeo Demonstrativo Blink Led Externo

Um vídeo demonstrativo do projeto em funcionamento está disponível no repositório (arquivo `ea`).

## 🎓 Conceitos Aprendidos

- ✅ Estrutura básica de um sketch Arduino (setup e loop)
- ✅ Configuração de pinos digitais
- ✅ Controle de saídas digitais (HIGH/LOW)
- ✅ Uso da função delay() para temporização
- ✅ Diferença entre LED_BUILTIN e pinos externos

## 🔍 Possíveis Expansões

- Alterar o tempo de delay para diferentes padrões de piscada
- Adicionar múltiplos LEDs com padrões diferentes
- Implementar efeitos como fade usando PWM
- Controlar o LED através de um botão
- Criar sequências de LEDs (efeito Knight Rider)

## 📚 Referências

- [Documentação Arduino](https://www.arduino.cc/reference/en/)
- [Tutorial Blink Oficial](https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink)
- [Tinkercad Circuits](https://www.tinkercad.com/circuits)

## 👤 Autor

Carlos Icaro, desenvolvido como parte do Módulo 4 de IOT do Instituto de Tecnologia e Liderança.

## 📄 Licença

Este projeto é de código aberto e está disponível para fins educacionais.
