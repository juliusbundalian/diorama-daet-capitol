#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Config.h"
#include "ControlLogic.h"
#include "ColorLogic.h"
#include "ButtonLadder.h"
#include "ShowLogic.h"

// Preliminary hardware map. Read README before enabling any output.
ControlLogic lights;
ShowLogic fountain;
WebServer server(80);
HardwareSerial player(2);
bool playing = false;
int rawButton = -1, stableButton = -1;
uint32_t buttonChangedAt = 0, streetUntil = 0, buildingUntil = 0;
uint32_t lastSense = 0, lastDisplay = 0, flagSince = 0;
uint32_t lastDiagnostics = 0;
int flagDirection = 0;
bool appliedPump1 = false, appliedPump2 = false;
uint32_t bothOffSince = 0;
rmt_data_t fountainPixels[24 * FOUNTAIN_PIXEL_COUNT];
rmt_data_t buildingPixels[24 * BUILDING_PIXEL_COUNT];
uint8_t detectedColor = 0;
const char *lastAction = "Ready";

// Common PCF8574 LCD backpack layout: RS=P0, E=P2, backlight=P3, D4..D7=P4..P7.
void lcdNibble(uint8_t nibble, bool rs) {
  uint8_t data = (nibble << 4) | 0x08 | (rs ? 1 : 0);
  Wire.beginTransmission(LCD_ADDRESS); Wire.write(data | 0x04); Wire.endTransmission();
  delayMicroseconds(1);
  Wire.beginTransmission(LCD_ADDRESS); Wire.write(data); Wire.endTransmission();
  delayMicroseconds(50);
}
void lcdByte(uint8_t value, bool rs = false) { lcdNibble(value >> 4, rs); lcdNibble(value & 15, rs); }
void lcdLine(uint8_t row, const char *text) {
  lcdByte(row ? 0xc0 : 0x80);
  bool ended = false;
  for (uint8_t i = 0; i < 16; ++i) {
    char c = ended ? ' ' : text[i];
    if (!c) { ended = true; c = ' '; }
    lcdByte(c, true);
  }
}
void lcdStart() {
  delay(50); lcdNibble(3, false); delay(5); lcdNibble(3, false);
  delayMicroseconds(200); lcdNibble(3, false); lcdNibble(2, false);
  lcdByte(0x28); lcdByte(0x0c); lcdByte(0x06); lcdByte(0x01); delay(2);
}
void playerCommand(uint8_t command, uint16_t value = 0) {
  uint8_t packet[10] = {0x7e,0xff,0x06,command,0,uint8_t(value >> 8),uint8_t(value),0,0,0xef};
  uint16_t sum = 0;
  for (int i = 1; i <= 6; ++i) sum += packet[i];
  sum = -sum; packet[7] = sum >> 8; packet[8] = sum;
  player.write(packet, 10);
}
void stopShow() { fountain.stop(); playerCommand(0x16); playing = false; lastAction = "Show stopped"; }
void command(uint8_t id) {
  if (id < 3) { lights.pressLight(id); lastAction = lights.automatic ? "Auto mode active" : "Light toggled"; }
  else if (id == 3) { lights.toggleMode(); lastAction = lights.automatic ? "Auto mode" : "Manual mode"; }
  else if (id == 4) {
    if (playing) stopShow();
    else { fountain.start(millis()); playerCommand(0x03, 1); playing = true; lastAction = "Show started"; }
  }
  Serial.println(lastAction);
}
void readButtons(uint32_t now) {
  int reading = decodeButtonMv(analogReadMilliVolts(PIN_BUTTON_ADC));
  if (reading != rawButton) buttonChangedAt = now;
  if (uint32_t(now - buttonChangedAt) >= 40 && reading != stableButton) {
    stableButton = reading;
    if (stableButton >= 0) command(stableButton);
  }
  rawButton = reading;
}
uint8_t sensedColor() {
  // TCS3200 S2/S3 filter selections, with S0/S1 fixed externally.
  const uint8_t pattern[3][2] = {{LOW, LOW}, {LOW, HIGH}, {HIGH, HIGH}}; // red, blue, green
  uint32_t period[3];
  for (int i = 0; i < 3; ++i) {
    digitalWrite(PIN_COLOR_S2, pattern[i][0]);
    digitalWrite(PIN_COLOR_S3, pattern[i][1]);
    delayMicroseconds(200);
    period[i] = pulseIn(PIN_COLOR_OUT, LOW, 1500);
    if (!period[i]) return 0;
  }
  return classifyColor(period[0], period[1], period[2]);
}
void readSensors(uint32_t now) {
  if (!lights.automatic || uint32_t(now - lastSense) < 200) return;
  lastSense = now;
  if (digitalRead(PIN_IR) == !IR_ACTIVE_LOW) streetUntil = now + 10000;
  if (!playing && digitalRead(PIN_SOUND) == !SOUND_ACTIVE_LOW) buildingUntil = now + 10000;
  detectedColor = sensedColor();
  lights.sensorLight(0, detectedColor != 0);
  lights.sensorLight(1, int32_t(streetUntil - now) > 0);
  lights.sensorLight(2, !playing && int32_t(buildingUntil - now) > 0);
}
void readPlayer() {
  while (player.available() >= 10) {
    if (player.read() != 0x7e) continue;
    uint8_t frame[9]; player.readBytes(frame, 9);
    if (frame[2] == 0x3d && playing) stopShow();
  }
}
void relay(int pin, bool on) { digitalWrite(pin, (on == RELAY_ACTIVE_LOW) ? LOW : HIGH); }
void drivePumps(uint32_t now) {
  fountain.update(now);
  if (!ENABLE_PUMP_OUTPUTS) return;
  auto want = fountain.outputs();
  if (appliedPump1 && !want.pump1) { relay(PIN_PUMP1, false); appliedPump1 = false; }
  if (appliedPump2 && !want.pump2) { relay(PIN_PUMP2, false); appliedPump2 = false; }
  if (!appliedPump1 && !appliedPump2) {
    if (!bothOffSince) bothOffSince = now;
    if (uint32_t(now - bothOffSince) >= 100) {
      if (want.pump1) { relay(PIN_PUMP1, true); appliedPump1 = true; }
      else if (want.pump2) { relay(PIN_PUMP2, true); appliedPump2 = true; }
    }
  } else bothOffSince = 0;
}
void driveFlag(uint32_t now) {
  if (!ENABLE_FLAG_MOTOR) return;
  int y = analogRead(PIN_JOY_Y);
  int requested = y < 1400 ? 1 : y > 2700 ? -1 : 0;
  if (requested != flagDirection) {
    digitalWrite(PIN_MOTOR_PWM, LOW);
    digitalWrite(PIN_MOTOR_IN1, LOW); digitalWrite(PIN_MOTOR_IN2, LOW);
    flagDirection = requested; flagSince = now;
  }
  if (requested && uint32_t(now - flagSince) < FLAG_MAX_RUN_MS) {
    digitalWrite(PIN_MOTOR_IN1, requested > 0);
    digitalWrite(PIN_MOTOR_IN2, requested < 0);
    digitalWrite(PIN_MOTOR_PWM, HIGH);
  } else digitalWrite(PIN_MOTOR_PWM, LOW);
}
void streetRgb() {
  if (!ENABLE_RGB_OUTPUTS) return;
  int levels[3] = {180, 120, 60};
  int pins[3] = {PIN_STREET_R, PIN_STREET_G, PIN_STREET_B};
  for (int i = 0; i < 3; ++i) {
    int value = lights.lights[1] ? levels[i] : 0;
    analogWrite(pins[i], STREET_COMMON_ANODE ? 255 - value : value);
  }
}
void stripWrite(int pin, rmt_data_t *bits, uint16_t count, uint8_t r, uint8_t g, uint8_t b) {
  const uint8_t color[3] = {g, r, b}; // WS2812-style GRB ordering; verify actual strip.
  unsigned index = 0;
  for (uint16_t led = 0; led < count; ++led) {
    for (uint8_t channel = 0; channel < 3; ++channel) {
      for (int8_t bit = 7; bit >= 0; --bit) {
        bool one = color[channel] & (1 << bit);
        bits[index].level0 = 1; bits[index].duration0 = one ? 8 : 4;
        bits[index].level1 = 0; bits[index].duration1 = one ? 4 : 8;
        ++index;
      }
    }
  }
  rmtWrite(pin, bits, index, RMT_WAIT_FOR_EVER);
}
void strips() {
  if (!ENABLE_RGB_OUTPUTS) return;
  static int lastF = -1, lastB = -1;
  int f = lights.lights[0] ? (lights.automatic ? detectedColor : 1) : 0;
  int b = lights.lights[2] ? 1 : 0;
  if (f != lastF) {
    stripWrite(PIN_FOUNTAIN_STRIP, fountainPixels, FOUNTAIN_PIXEL_COUNT,
               f == 1 ? 80 : 0, f == 3 ? 80 : 0, f == 2 ? 80 : 0);
    lastF = f;
  }
  if (b != lastB) {
    stripWrite(PIN_BUILDING_STRIP, buildingPixels, BUILDING_PIXEL_COUNT,
               b ? 80 : 0, b ? 80 : 0, b ? 80 : 0);
    lastB = b;
  }
}
void display(uint32_t now) {
  if (uint32_t(now - lastDisplay) < 1000) return;
  lastDisplay = now;
  char line[17];
  snprintf(line, sizeof(line), "%s %s", lights.automatic ? "AUTO" : "MANUAL", WiFi.softAPgetStationNum() ? "APP ON" : "APP OFF");
  lcdLine(0, line);
  if ((now / 4000) % 2) lcdLine(1, lastAction);
  else {
    snprintf(line, sizeof(line), "F%d S%d B%d P%d", lights.lights[0], lights.lights[1], lights.lights[2], playing);
    lcdLine(1, line);
  }
}
void diagnostics(uint32_t now) {
  if (uint32_t(now - lastDiagnostics) < 2000) return;
  lastDiagnostics = now;
  Serial.printf("JOY Y=%d BUTTON=%d mV IR=%d SOUND=%d COLOR=%u MODE=%s PUMPS=%d/%d\n",
                analogRead(PIN_JOY_Y), analogReadMilliVolts(PIN_BUTTON_ADC),
                digitalRead(PIN_IR), digitalRead(PIN_SOUND), detectedColor,
                lights.automatic ? "AUTO" : "MANUAL", appliedPump1, appliedPump2);
}

void setupWeb() {
  WiFi.mode(WIFI_AP); WiFi.softAP("DaetCapitol", "DaetCapitol2026");
  server.on("/", []() {
    String html = "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'><title>Daet Capitol</title>"
      "<style>body{font:18px system-ui;background:#17363a;color:#fff;max-width:440px;margin:30px auto;padding:16px}button{display:block;width:100%;padding:15px;margin:12px 0;border:0;border-radius:10px;background:#e3b967;color:#172e30;font-size:18px}</style>"
      "<h1>Daet Capitol</h1><p id=status>Connecting...</p>"
      "<button onclick='send(0)'>Fountain lights</button><button onclick='send(1)'>Streetlights</button><button onclick='send(2)'>Building lights</button>"
      "<button onclick='send(3)'>Manual / automatic</button><button onclick='send(4)'>Audio and fountain</button>"
      "<script>function send(n){fetch('/command?n='+n).then(update)}function update(){fetch('/state').then(r=>r.json()).then(x=>document.getElementById('status').textContent=x.mode+' | Lights '+x.lights+' | Show '+x.show)}update();setInterval(update,2000)</script>";
    server.send(200, "text/html", html);
  });
  server.on("/command", []() {
    String n = server.arg("n");
    if (n.length() != 1 || n[0] < '0' || n[0] > '4') { server.send(400, "text/plain", "Invalid command"); return; }
    command(n[0] - '0'); server.send(200, "text/plain", "OK");
  });
  server.on("/state", []() {
    String body = String("{\"mode\":\"") + (lights.automatic ? "Automatic" : "Manual") +
      "\",\"lights\":\"" + String(lights.lights[0]) + String(lights.lights[1]) + String(lights.lights[2]) +
      "\",\"show\":" + (playing ? "true" : "false") + "}";
    server.send(200, "application/json", body);
  });
  server.begin();
  Serial.print("Phone page: http://"); Serial.println(WiFi.softAPIP());
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  lcdStart();
  pinMode(PIN_COLOR_OUT, INPUT);
  pinMode(PIN_JOY_Y, INPUT); pinMode(PIN_BUTTON_ADC, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_BUTTON_ADC, ADC_11db);
  pinMode(PIN_IR, INPUT); pinMode(PIN_SOUND, INPUT);
  pinMode(PIN_COLOR_S2, OUTPUT); pinMode(PIN_COLOR_S3, OUTPUT);
  if (ENABLE_PUMP_OUTPUTS) {
    relay(PIN_PUMP1, false); relay(PIN_PUMP2, false);
    pinMode(PIN_PUMP1, OUTPUT); pinMode(PIN_PUMP2, OUTPUT);
  }
  if (ENABLE_FLAG_MOTOR) {
    digitalWrite(PIN_MOTOR_PWM, LOW); pinMode(PIN_MOTOR_PWM, OUTPUT);
    pinMode(PIN_MOTOR_IN1, OUTPUT); pinMode(PIN_MOTOR_IN2, OUTPUT);
  }
  if (ENABLE_RGB_OUTPUTS) {
    pinMode(PIN_STREET_R, OUTPUT); pinMode(PIN_STREET_G, OUTPUT); pinMode(PIN_STREET_B, OUTPUT);
    rmtInit(PIN_FOUNTAIN_STRIP, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);
    rmtInit(PIN_BUILDING_STRIP, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);
  }
  player.begin(9600, SERIAL_8N1, PIN_AUDIO_RX, PIN_AUDIO_TX);
  setupWeb(); Serial.println("Daet Capitol ready");
}

void loop() {
  uint32_t now = millis();
  server.handleClient(); readButtons(now); readSensors(now); readPlayer();
  drivePumps(now); driveFlag(now); streetRgb(); strips(); display(now); diagnostics(now);
}
