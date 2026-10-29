#include <Wire.h>

constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Serial.println("LCD I2C diagnostic: SDA=GPIO21, SCL=GPIO22");
}

void loop() {
  uint8_t found = 0;

  for (uint8_t address = 0x08; address <= 0x77; ++address) {
    Wire.beginTransmission(address);
    const uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("Device found at 0x%02X\n", address);
      ++found;
    }
  }

  if (found == 0) Serial.println("No I2C device found");
  delay(3000);
}
