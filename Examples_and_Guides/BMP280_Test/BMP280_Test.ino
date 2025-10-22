/*
  BMP280_Test.ino — Robust test sketch for BMP280 over I2C

  What it does
  - Scans the I2C bus and prints any detected addresses
  - Tries to initialize the BMP280 at 0x76 and 0x77 (both common)
  - Prints sensor ID and detects if you actually have a BME280
  - Reads temperature, pressure and computes altitude

  Wiring (I2C)
  - BMP280 VCC  -> 3.3V (prefer) or 5V (if breakout supports it)
  - BMP280 GND  -> GND
  - BMP280 SCL  -> Arduino SCL (A5 on Uno/Nano; D21 on MEGA; 22 on ESP32 dev; 15 on ESP8266)
  - BMP280 SDA  -> Arduino SDA (A4 on Uno/Nano; D20 on MEGA; 21 on ESP32 dev; 4 on ESP8266)
  - Keep I2C pull-ups: many breakout boards already have 4.7k to 3.3V

  Notes
  - Some boards ship configured for address 0x76, others for 0x77
  - If sensorID == 0x60, you have a BME280 (use Adafruit_BME280 library)
  - If nothing is found, re-check wiring and power; run-time output shows hints

  Baud rate: 115200
*/

#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // I2C instance

// Sea level pressure (hPa) used to compute altitude; adjust to your local
// weather for accurate altitude. 1013.25 hPa is the standard.
const float SEALEVEL_HPA = 1013.25;

void scanI2C() {
  Serial.println(F("\n-- I2C Scan --"));
  byte count = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print(F("Found I2C device at 0x"));
      if (addr < 16) Serial.print('0');
      Serial.print(addr, HEX);
      Serial.println();
      count++;
    }
  }
  if (count == 0) Serial.println(F("No I2C devices found"));
  Serial.println(F("-- End Scan --\n"));
}

bool initBMP280() {
  // Try common addresses: 0x76 then 0x77
  const uint8_t addrs[] = {0x76, 0x77};
  for (uint8_t i = 0; i < 2; i++) {
    uint8_t addr = addrs[i];
    Serial.print(F("Trying BMP280 at 0x"));
    Serial.println(addr, HEX);
    if (bmp.begin(addr)) {
      Serial.print(F("BMP280 initialized at 0x"));
      Serial.println(addr, HEX);
      return true;
    }
  }

  // If default tries fail, print sensorID for hints
  Serial.println(F("Could not initialize BMP280 at 0x76 or 0x77."));
  Serial.print(F("Raw sensorID: 0x"));
  Serial.println(bmp.sensorID(), HEX);
  Serial.println(F("Hints: 0x60=BME280 (use BME library), 0x56..0x58=BMP280, 0xFF=bad address"));
  return false;
}

void setup() {
  Serial.begin(115200);
  // On boards with native USB, give time for Serial to open
#if defined(USBCON) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
  while (!Serial) { ; }
#endif

  Serial.println(F("\n=== BMP280 Test ==="));
  Wire.begin();

  scanI2C();

  if (!initBMP280()) {
    Serial.println(F("\nERROR: BMP280 not detected. Check wiring (VCC, GND, SDA, SCL), address jumper, and power."));
    Serial.println(F("If sensorID is 0x60, install Adafruit_BME280 and try a BME example."));
    while (true) delay(100);
  }

  // Configure sampling similar to datasheet recommendations
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating mode
                  Adafruit_BMP280::SAMPLING_X2,     // Temperature oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // IIR filter
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time

  // Extra sensor info
  uint8_t id = bmp.sensorID();
  Serial.print(F("Detected sensorID: 0x"));
  Serial.println(id, HEX);
  if (id == 0x60) {
    Serial.println(F("Note: 0x60 indicates a BME280 (with humidity). This sketch only uses BMP280 features."));
  }
}

void loop() {
  float T = bmp.readTemperature();          // °C
  float P = bmp.readPressure() / 100.0F;    // hPa
  float alt = bmp.readAltitude(SEALEVEL_HPA); // meters

  Serial.print(F("Temperature: "));
  Serial.print(T);
  Serial.println(F(" °C"));

  Serial.print(F("Pressure: "));
  Serial.print(P);
  Serial.println(F(" hPa"));

  Serial.print(F("Approx. altitude: "));
  Serial.print(alt);
  Serial.println(F(" m"));

  Serial.println();
  delay(2000);
}
