/*
  Código de Debug para Sensor PIR
  Este código ajuda a configurar e testar o sensor PIR
  
  Funcionalidades:
  - Lê o valor do PIR continuamente
  - Imprime o valor no Serial Monitor
  - Acende LED verde quando detecta movimento
  - Mostra estatísticas de detecção
*/

// Includes necessários para ESP32
#include "driver/rtc_io.h"

// ==== Configurações de Hardware ====
#define PIR_PIN 12        // GPIO 12 - Pino do sensor PIR
#define LED_GREEN_PIN 15  // GPIO 15 - LED verde

// ==== Variáveis ====
int pirState = LOW;
int lastPirState = LOW;
unsigned long lastChangeTime = 0;
unsigned long detectionCount = 0;
unsigned long lastDetectionTime = 0;

void setup() {
  // Inicializar Serial Monitor
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("========================================");
  Serial.println("  DEBUG DO SENSOR PIR");
  Serial.println("========================================");
  Serial.println();
  
  // Configurar pinos
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  
  // Habilitar pull-down no pino do PIR para evitar leituras flutuantes
  // Isso é importante quando o PIR não está conectado
  rtc_gpio_init((gpio_num_t)PIR_PIN);
  rtc_gpio_set_direction((gpio_num_t)PIR_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pulldown_en((gpio_num_t)PIR_PIN);
  rtc_gpio_pullup_dis((gpio_num_t)PIR_PIN);
  
  // Verificar estado inicial
  delay(500);
  int initialReading = digitalRead(PIR_PIN);
  
  Serial.print("Estado inicial do PIR: ");
  Serial.println(initialReading == HIGH ? "HIGH (Movimento detectado)" : "LOW (Sem movimento)");
  Serial.println();
  
  if (initialReading == HIGH) {
    Serial.println("AVISO: PIR está em HIGH na inicialização!");
    Serial.println("Verifique se o sensor está conectado corretamente.");
    Serial.println();
  }
  
  Serial.println("Iniciando monitoramento...");
  Serial.println("Formato: [Tempo] PIR=LOW/HIGH | LED=ON/OFF | Detecções: X");
  Serial.println("----------------------------------------");
  Serial.println();
  
  // Desligar LED inicialmente
  digitalWrite(LED_GREEN_PIN, LOW);
  
  lastPirState = initialReading;
  lastChangeTime = millis();
}

void loop() {
  // Ler estado atual do PIR
  int currentReading = digitalRead(PIR_PIN);
  
  // Verificar se houve mudança de estado
  if (currentReading != lastPirState) {
    lastChangeTime = millis();
    lastPirState = currentReading;
    
    if (currentReading == HIGH) {
      detectionCount++;
      lastDetectionTime = millis();
      Serial.print("[");
      Serial.print(millis() / 1000);
      Serial.print("s] ");
      Serial.println(">>> MOVIMENTO DETECTADO! <<<");
    } else {
      Serial.print("[");
      Serial.print(millis() / 1000);
      Serial.print("s] ");
      Serial.println("Movimento cessou.");
    }
  }
  
  // Atualizar LED
  if (currentReading == HIGH) {
    digitalWrite(LED_GREEN_PIN, HIGH);
  } else {
    digitalWrite(LED_GREEN_PIN, LOW);
  }
  
  // Imprimir status a cada segundo (para não poluir o Serial Monitor)
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime >= 1000) {
    lastPrintTime = millis();
    
    Serial.print("[");
    Serial.print(millis() / 1000);
    Serial.print("s] ");
    Serial.print("PIR=");
    Serial.print(currentReading == HIGH ? "HIGH" : "LOW");
    Serial.print(" | LED=");
    Serial.print(currentReading == HIGH ? "ON " : "OFF");
    Serial.print(" | Detecções: ");
    Serial.print(detectionCount);
    
    if (currentReading == HIGH) {
      unsigned long duration = millis() - lastChangeTime;
      Serial.print(" | Duração: ");
      Serial.print(duration / 1000);
      Serial.print("s");
    }
    
    Serial.println();
  }
  
  // Sem delay - leitura contínua para máxima responsividade
  // O Serial.print já tem overhead suficiente para não sobrecarregar
}

