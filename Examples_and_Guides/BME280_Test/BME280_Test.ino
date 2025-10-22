/*
  BME280_Test.ino — Working test for BME280 sensor over I2C

  What it does
  - Initializes BME280 at address 0x76 (detected from your I2C scan)
  - Reads temperature, humidity, pressure and computes altitude
  - Prints all values to Serial every 2 seconds

  Your sensor info
  - I2C Address: 0x76
  - Sensor ID: 0x60 (BME280 with humidity sensor)

  Wiring (I2C)
  - BME280 VCC  -> 3.3V (recommended) or 5V
  - BME280 GND  -> GND
  - BME280 SCL  -> Pin 22 (ESP32) / A5 (Uno/Nano)
  - BME280 SDA  -> Pin 21 (ESP32) / A4 (Uno/Nano)

  Library required
  - Adafruit BME280 Library (install via Library Manager)
  - Adafruit Unified Sensor (dependency, auto-installed)

  Baud rate: 115200
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Your sensor is at 0x76
#define BME280_ADDRESS 0x76

Adafruit_BME280 bme; // I2C instance

// Sea level pressure (hPa) - adjust for your location for accurate altitude
const float SEALEVEL_HPA = 1013.25;

void setup() {
  Serial.begin(115200);
  
  // Wait for Serial on native USB boards
#if defined(USBCON) || defined(ARDUINO_ARCH_ESP32)
  while (!Serial && millis() < 5000) { ; }
#endif

  Serial.println(F("\n=== BME280 Test (Sensor ID: 0x60) ==="));
  
  Wire.begin();
  
  // Initialize BME280 at your detected address
  Serial.print(F("Initializing BME280 at 0x"));
  Serial.println(BME280_ADDRESS, HEX);
  
  if (!bme.begin(BME280_ADDRESS)) {
    Serial.println(F("ERROR: Could not find BME280 sensor at 0x76!"));
    Serial.println(F("Check wiring (VCC, GND, SDA, SCL) and power."));
    while (1) delay(100);
  }
  
  Serial.println(F("BME280 initialized successfully!"));
  Serial.print(F("Sensor ID: 0x"));
  Serial.println(bme.sensorID(), HEX);
  
  // Configure sensor for normal mode with recommended settings
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X2,  // Temperature oversampling
                  Adafruit_BME280::SAMPLING_X16, // Pressure oversampling
                  Adafruit_BME280::SAMPLING_X1,  // Humidity oversampling
                  Adafruit_BME280::FILTER_X16,   // IIR filter
                  Adafruit_BME280::STANDBY_MS_500);
  
  Serial.println(F("\nStarting readings...\n"));
  delay(100);
}

void loop() {
  // Read all sensor values
  float temperature = bme.readTemperature();        // °C
  float humidity = bme.readHumidity();              // %
  float pressure = bme.readPressure() / 100.0F;     // hPa
  float altitude = bme.readAltitude(SEALEVEL_HPA);  // meters

  // Print formatted output
  Serial.println(F("--- BME280 Readings ---"));
  
  Serial.print(F("Temperature: "));
  Serial.print(temperature, 2);
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
  
  delay(2000); // Wait 2 seconds between readings
}
