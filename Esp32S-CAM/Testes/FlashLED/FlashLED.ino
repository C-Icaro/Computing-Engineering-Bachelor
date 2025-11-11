#include <Arduino.h>

constexpr gpio_num_t FLASH_LED_PIN = GPIO_NUM_4;

void setup()
{
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("=== TESTE FLASH LED ESP32-CAM ==="));
}

void loop()
{
  Serial.println(F("LED ON"));
  digitalWrite(FLASH_LED_PIN, HIGH);
  delay(1000);

  Serial.println(F("LED OFF"));
  digitalWrite(FLASH_LED_PIN, LOW);
  delay(1000);
}


