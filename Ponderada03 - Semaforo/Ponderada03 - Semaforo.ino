// Semáforo em ESP32 (ESP-WROOM-32U)
// Pinos conforme Readme.md
//  - Verde  : 27
//  - Amarelo: 26
//  - Vermelho: 25

#include <Arduino.h>

class Led {
public:
  explicit Led(uint8_t pinNumber) : pin(pinNumber) {}

  void begin() {
    pinMode(pin, OUTPUT);
    off();
  }

  void on() { digitalWrite(pin, HIGH); }
  void off() { digitalWrite(pin, LOW); }

private:
  uint8_t pin;
};

enum class LightState { Green, Yellow, Red };

class TrafficLight {
public:
  TrafficLight(Led* greenLed,
               Led* yellowLed,
               Led* redLed,
               unsigned long greenDurationMs,
               unsigned long yellowDurationMs,
               unsigned long redDurationMs)
      : green(greenLed),
        yellow(yellowLed),
        red(redLed),
        greenMs(greenDurationMs),
        yellowMs(yellowDurationMs),
        redMs(redDurationMs),
        state(LightState::Red),
        lastChange(0) {}

  void begin() {
    if (green) green->begin();
    if (yellow) yellow->begin();
    if (red) red->begin();
    setState(LightState::Red);
  }

  void update() {
    const unsigned long now = millis();
    const unsigned long elapsed = now - lastChange;

    switch (state) {
      case LightState::Green:
        if (elapsed >= greenMs) {
          setState(LightState::Yellow);
        }
        break;
      case LightState::Yellow:
        if (elapsed >= yellowMs) {
          setState(LightState::Red);
        }
        break;
      case LightState::Red:
        if (elapsed >= redMs) {
          setState(LightState::Green);
        }
        break;
    }
  }

private:
  void setState(LightState next) {
    // Desliga todos antes de ligar o próximo estado
    if (green) green->off();
    if (yellow) yellow->off();
    if (red) red->off();

    state = next;
    lastChange = millis();

    if (state == LightState::Green) {
      if (green) green->on();
    } else if (state == LightState::Yellow) {
      if (yellow) yellow->on();
    } else { // Red
      if (red) red->on();
    }
  }

  Led* green;
  Led* yellow;
  Led* red;
  unsigned long greenMs;
  unsigned long yellowMs;
  unsigned long redMs;
  LightState state;
  unsigned long lastChange;
};

// Ponteiros para demonstrar uso de POO + ponteiros
Led* ledGreen = nullptr;
Led* ledYellow = nullptr;
Led* ledRed = nullptr;
TrafficLight* trafficLight = nullptr;

void setup() {
  // Inicializa serial (opcional para debug)
  Serial.begin(115200);
  delay(50);
  Serial.println("Semaforo iniciado");

  // Cria objetos dinamicamente (uso explícito de ponteiros)
  ledGreen = new Led(27);
  ledYellow = new Led(26);
  ledRed = new Led(25);

  // Durações conforme desafio (Butantã): Vermelho 6s, Verde 4s, Amarelo 2s
  trafficLight = new TrafficLight(ledGreen, ledYellow, ledRed,
                                  4000UL, 2000UL, 6000UL);

  trafficLight->begin();
}

void loop() {
  if (trafficLight) {
    trafficLight->update();
  }
}


