#pragma once

// Set these only after verifying the electrical interface and power ratings.
constexpr bool ENABLE_PUMP_OUTPUTS = false;
constexpr bool ENABLE_FLAG_MOTOR = false;
constexpr bool ENABLE_RGB_OUTPUTS = false;

constexpr int PIN_I2C_SDA = 21, PIN_I2C_SCL = 22;
constexpr int PIN_JOY_Y = 34, PIN_BUTTON_ADC = 39;
constexpr int PIN_COLOR_OUT = 36;
constexpr int PIN_IR = 35, PIN_SOUND = 26;
constexpr int PIN_COLOR_S2 = 5, PIN_COLOR_S3 = 25;
constexpr int PIN_MOTOR_IN1 = 32, PIN_MOTOR_IN2 = 33, PIN_MOTOR_PWM = 23;
constexpr int PIN_PUMP1 = 18, PIN_PUMP2 = 19;
constexpr int PIN_AUDIO_RX = 16, PIN_AUDIO_TX = 17;
constexpr int PIN_STREET_R = 13, PIN_STREET_G = 14, PIN_STREET_B = 12;
constexpr int PIN_FOUNTAIN_STRIP = 4, PIN_BUILDING_STRIP = 27;

constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr bool RELAY_ACTIVE_LOW = true; // Verify with a disconnected pump first.
constexpr bool STREET_COMMON_ANODE = false; // Confirm RGB LED polarity.
constexpr bool IR_ACTIVE_LOW = true, SOUND_ACTIVE_LOW = true; // Verify module outputs.
constexpr uint16_t FOUNTAIN_PIXEL_COUNT = 30; // Replace with actual count.
constexpr uint16_t BUILDING_PIXEL_COUNT = 30; // Replace with actual count.
constexpr uint32_t FLAG_MAX_RUN_MS = 5000; // Calibrate against physical travel.i d

// TCS3200 S0/S1 require verified fixed frequency scaling; OE enabled.
// GPIO5 and GPIO12 are boot-strapping pins. Check attached circuit states
// at power-up before enabling RGB outputs or attaching the color sensor.
