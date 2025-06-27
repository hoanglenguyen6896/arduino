#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>

Adafruit_BMP280 bmp; // I2C interface for BMP280
Adafruit_BME280 bme; // I2C interface for BME280

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Try to initialize BMP280
  if (bmp.begin(0x76) || bmp.begin(0x77)) {
    Serial.println("Found BMP280 sensor");
  } else {
    // Try to initialize BME280 if BMP280 is not found
    if (bme.begin(0x76) || bme.begin(0x77)) {
      Serial.println("Found BME280 sensor");
    } else {
      Serial.println("No BMP280 or BME280 sensor found");
    }
  }
}

void loop() {
  // No need to do anything in the loop
}
