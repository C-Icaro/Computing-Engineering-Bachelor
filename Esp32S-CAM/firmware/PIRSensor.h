#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <Arduino.h>
#include "config.h"

// Ativar debug detalhado do PIR (descomente para ativar)
#define PIR_DEBUG_ENABLED true

class PIRSensor
{
public:
  PIRSensor(int pin = PIR_GPIO_PIN) : pirPin(pin), lastMotionTime(0), lastState(false), 
                                      stateChangeCount(0), lastDebugTime(0) {}

  void begin()
  {
    pinMode(pirPin, INPUT);
    lastState = digitalRead(pirPin) == HIGH;
    lastMotionTime = 0;
    stateChangeCount = 0;
    lastDebugTime = 0;
    Serial.printf("[PIR] Sensor inicializado no pino GPIO %d\n", pirPin);
    Serial.printf("[PIR] Estado inicial: %s\n", lastState ? "HIGH (ativo)" : "LOW (inativo)");
    Serial.printf("[PIR] Debounce configurado: %lu ms\n", MOTION_DEBOUNCE_MS);
    #if PIR_DEBUG_ENABLED
    Serial.println("[PIR] Debug detalhado ATIVADO");
    #endif
  }

  bool isMotionDetected()
  {
    bool currentState = digitalRead(pirPin) == HIGH;
    unsigned long now = millis();

    #if PIR_DEBUG_ENABLED
    // Log periódico do estado atual (a cada 5 segundos)
    if (now - lastDebugTime > 5000)
    {
      lastDebugTime = now;
      Serial.printf("[PIR-DEBUG] Estado atual: %s | Última detecção há: %lu ms | Total de mudanças: %u\n",
                    currentState ? "HIGH" : "LOW",
                    lastMotionTime > 0 ? (now - lastMotionTime) : 0,
                    stateChangeCount);
    }
    #endif

    // Detectar mudança de estado (LOW -> HIGH ou HIGH -> LOW)
    if (currentState != lastState)
    {
      stateChangeCount++;
      #if PIR_DEBUG_ENABLED
      Serial.printf("[PIR-DEBUG] Mudança de estado detectada: %s -> %s (mudança #%u)\n",
                    lastState ? "HIGH" : "LOW",
                    currentState ? "HIGH" : "LOW",
                    stateChangeCount);
      #endif
    }

    // Detectar transição LOW -> HIGH (movimento detectado)
    if (currentState && !lastState)
    {
      unsigned long timeSinceLastMotion = now - lastMotionTime;
      
      #if PIR_DEBUG_ENABLED
      Serial.printf("[PIR-DEBUG] Transição LOW->HIGH detectada! Tempo desde última: %lu ms\n", 
                    timeSinceLastMotion);
      #endif

      // Debounce: verificar se passou tempo suficiente desde última detecção
      if (timeSinceLastMotion > MOTION_DEBOUNCE_MS)
      {
        lastMotionTime = now;
        lastState = currentState;
        Serial.println("========================================");
        Serial.printf("[PIR] ✓✓✓ MOVIMENTO DETECTADO! ✓✓✓\n");
        Serial.printf("[PIR] Timestamp: %lu ms\n", now);
        Serial.printf("[PIR] Tempo desde última detecção: %lu ms\n", timeSinceLastMotion);
        Serial.printf("[PIR] Total de mudanças de estado: %u\n", stateChangeCount);
        Serial.println("========================================");
        return true;
      }
      else
      {
        #if PIR_DEBUG_ENABLED
        Serial.printf("[PIR-DEBUG] Movimento ignorado (debounce): %lu ms < %lu ms\n",
                      timeSinceLastMotion, MOTION_DEBOUNCE_MS);
        #endif
      }
    }

    lastState = currentState;
    return false;
  }

  unsigned long getLastMotionTime() const
  {
    return lastMotionTime;
  }

  bool getCurrentState() const
  {
    return digitalRead(pirPin) == HIGH;
  }

  void printStatus() const
  {
    unsigned long now = millis();
    bool currentState = digitalRead(pirPin) == HIGH;
    Serial.println("========== STATUS DO SENSOR PIR ==========");
    Serial.printf("Pino GPIO: %d\n", pirPin);
    Serial.printf("Estado atual: %s (%s)\n", 
                  currentState ? "HIGH" : "LOW",
                  currentState ? "ATIVO (movimento detectado)" : "INATIVO (sem movimento)");
    Serial.printf("Última detecção: %lu ms atrás\n", 
                  lastMotionTime > 0 ? (now - lastMotionTime) : 0);
    Serial.printf("Total de mudanças de estado: %u\n", stateChangeCount);
    Serial.printf("Debounce configurado: %lu ms\n", MOTION_DEBOUNCE_MS);
    Serial.println("==========================================");
  }

private:
  int pirPin;
  unsigned long lastMotionTime;
  bool lastState;
  unsigned int stateChangeCount;
  unsigned long lastDebugTime;
};

#endif // PIR_SENSOR_H



