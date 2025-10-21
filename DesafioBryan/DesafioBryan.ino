/***************************************************************************
  BME280 Test - Temperature, Humidity, Pressure & Altitude

  Your sensor is a BME280 (sensor ID: 0x60), not BMP280
  BME280 includes humidity sensing in addition to temperature and pressure

  I2C Address: 0x76 (detected from scan)

  Wiring (I2C)
  - BME280 VCC  -> 3.3V or 5V
  - BME280 GND  -> GND
  - BME280 SCL  -> Pin 22 (ESP32) / A5 (Uno)
  - BME280 SDA  -> Pin 21 (ESP32) / A4 (Uno)

  Library required: Adafruit BME280 Library
 ***************************************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME280_ADDRESS 0x76

Adafruit_BME280 bme; // I2C

void setup() {
  Serial.begin(115200);
  
#if defined(ARDUINO_ARCH_ESP32)
  while (!Serial && millis() < 5000) { ; }
#endif
  
  Serial.println(F("\n=== BME280 Test ==="));
  Wire.begin();
  
  if (!bme.begin(BME280_ADDRESS)) {
    Serial.println(F("ERROR: Could not find BME280 sensor at 0x76!"));
    Serial.print(F("SensorID was: 0x")); 
    Serial.println(bme.sensorID(), HEX);
    Serial.println(F("Check wiring (VCC, GND, SDA, SCL) and power."));
    while (1) delay(100);
  }

  Serial.println(F("BME280 initialized successfully!"));
  Serial.print(F("Sensor ID: 0x"));
  Serial.println(bme.sensorID(), HEX);

  // Configure sensor settings
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X2,     // Temperature
                  Adafruit_BME280::SAMPLING_X16,    // Pressure
                  Adafruit_BME280::SAMPLING_X1,     // Humidity
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_500);
  
  Serial.println(F("\nStarting readings...\n"));
}

void loop() {
    float temp = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;  // Convert to hPa
    float altitude = bme.readAltitude(1013.25);

    Serial.println(F("--- BME280 Readings ---"));
    
    Serial.print(F("Temperature: "));
    Serial.print(temp, 2);
    Serial.println(F(" °C"));

    Serial.print(F("Humidity: "));
    Serial.print(humidity, 2);
    Serial.println(F(" %"));

    Serial.print(F("Pressure: "));
    Serial.print(pressure, 2);
    Serial.println(F(" hPa"));

    Serial.print(F("Approx. Altitude: "));
    Serial.print(altitude, 2);
    Serial.println(F(" m"));

    Serial.println();
    delay(2000);
}
