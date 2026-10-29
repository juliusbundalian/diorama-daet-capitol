#include <Wire.h>

// Common PCF8574 backpack mapping. No extra Arduino library is required.
constexpr uint8_t ADDRESS = 0x27;
void nibble(uint8_t value, bool text) {
  uint8_t data = (value << 4) | 0x08 | (text ? 1 : 0);
  Wire.beginTransmission(ADDRESS); Wire.write(data | 0x04); Wire.endTransmission();
  delayMicroseconds(1);
  Wire.beginTransmission(ADDRESS); Wire.write(data); Wire.endTransmission();
  delayMicroseconds(50);
}
void sendByte(uint8_t value, bool text = false) {
  nibble(value >> 4, text); nibble(value & 15, text);
}
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  delay(50);
  nibble(3, false); delay(5); nibble(3, false);
  delayMicroseconds(200); nibble(3, false); nibble(2, false);
  sendByte(0x28); sendByte(0x0c); sendByte(0x06); sendByte(0x01); delay(2);
  sendByte(0x80);
  for (const char *p = "Daet Capitol"; *p; ++p) sendByte(*p, true);
  sendByte(0xc0);
  for (const char *p = "LCD test OK"; *p; ++p) sendByte(*p, true);
  Serial.println("Text sent to LCD at 0x27");
}
void loop() {}
