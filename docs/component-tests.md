# Component bench tests

Test one component at a time on the 38-pin NodeMCU ESP-32S v1.1. Keep pumps and the flag motor disconnected during sensor and display tests. Record the observed result before adding the next part.

## 1. I2C LCD — find its address

Open `firmware/diagnostics/LcdI2CScan/LcdI2CScan.ino` in Arduino IDE. Select the appropriate ESP32 board and USB port, upload, and open Serial Monitor at 115200 baud. The scanner uses P21/GPIO21 for SDA and P22/GPIO22 for SCL. It should report an address, often 0x27 or 0x3F, but record the address actually found. **Observed on the user's board: 0x27 repeatedly.**

LCD connection: GND to the ESP32/common ground; SDA to GPIO21; SCL to GPIO22; VCC to the voltage confirmed for the LCD module. **If the LCD backpack is powered at 5 V, check its I2C pull-up voltage and use a suitable bidirectional I2C level shifter before connecting SDA/SCL to the 3.3 V ESP32.** Do not connect 5 V SDA/SCL directly to ESP32 GPIO.

If Serial Monitor says `No I2C device found`, first check that SDA and SCL are not swapped, grounds are common, the LCD has power, and the voltage interface is correct.

### Display text

Open `firmware/diagnostics/LcdTextTest/LcdTextTest.ino` and upload with the same wiring; no extra library is needed. The expected display is `Daet Capitol` on row 1 and `LCD test OK` on row 2. If the backlight is on but text is invisible, adjust the small contrast screw on the LCD backpack. If text still does not appear, the backpack may have a different pin mapping despite responding at 0x27; record its chip marking and wiring.

## Remaining sequence

1. Five buttons and joystick: read inputs over Serial Monitor and confirm stable pressed/released and centered/up/down readings.
2. IR, sound, and color sensors: read raw values and calibrate thresholds under actual diorama lighting and speaker conditions.
3. Audio module and speaker: test playback separately from the microphone.
4. Four shared-color streetlights: verify common-pin polarity and current limiting before driving all four.
5. Two 1 m addressable strips: identify the chipset and pixel count, then test each strip alone on the sized 5 V branch.
6. Flag motor and TB6612FNG: test direction, joystick stop, and a calibrated maximum run time without pumps attached.
7. Two pump channels: check relay idle state without pumps, then run each pump under its approved electrical and operating conditions before alternating them.

## Complete firmware acceptance checklist

Use `firmware/DaetCapitol/DaetCapitol.ino` and its README. Keep `ENABLE_PUMP_OUTPUTS`, `ENABLE_FLAG_MOTOR`, and `ENABLE_RGB_OUTPUTS` false until the associated wiring and ratings are confirmed. Record pass/fail, observed result, and any revised thresholds. Do not infer physical operation from a successful compile.

| Check | Action | Expected result |
| --- | --- | --- |
| Boot | Power the ESP32 with loads disconnected | Serial says ready; all controlled outputs remain off; LCD shows manual mode. |
| Phone page | Join `DaetCapitol` WiFi, open `http://192.168.4.1` | Page loads, reports manual mode and show off; LCD reports app connection after phone joins. |
| I2C LCD | Inspect both rows | Mode/status and latest action alternate; no blank display or corrupted characters. |
| Five buttons | Build the resistor ladder in the firmware README; press each once, never together | Serial `BUTTON` readings form five distinct levels; corresponding action appears on LCD/Serial; one press causes one action. |
| Manual lights | Press each lighting button twice | Only its lighting group toggles on and back off. |
| Mode switch | Enter automatic, press a lighting button, return to manual | Sensor mode owns lights; manual light button has no effect in automatic mode. |
| Joystick input | Move up, center, down with motor output disabled | The firmware's `JOY Y` Serial reading changes across center and extremes; calibrate thresholds in firmware. |
| IR sensor | Enter automatic mode and trigger IR module | Streetlight state turns on and stays on 10 seconds after last trigger. |
| Sound sensor | Trigger microphone without audio playback | Building light state turns on and stays on 10 seconds after last trigger. |
| Audio feedback | Start onboard audio, observe sound sensor behavior | Onboard audio does not turn on building lighting. |
| Color sensor | Present red, blue, green, then no target | Fountain strip follows calibrated colors; no target turns it off. Current code uses a provisional relative-frequency rule, so this test determines calibration changes. |
| Audio module | Press show with pumps disabled | Track 1 plays; a second press stops it. Verify end-of-track event stops show state. |
| Streetlights | After confirming polarity/drivers, enable RGB outputs | All four lamps share the same color and switch together. |
| Two strips | After confirming chipset/count/data shifter/5 V power, enable RGB outputs | Fountain and building strips respond separately; first and last pixels show correct colors. |
| Flag motor | After confirming supply/TB6612/physical stop, enable motor | Joystick directions are correct, center stops, and continuous-run timeout stops motion. |
| Relay idle | After checking relay polarity, enable pump outputs with pumps still disconnected | Both contacts stay open on boot and reset. |
| Pump sequence | Connect pumps under their rated conditions and start show | Pump 1 runs for 5 seconds, then pump 2 for 5 seconds, repeating; neither runs at the same time. |
| Stop and reset | Stop show during each phase, then reset while a phase is active | Both pumps turn off; startup does not resume them. |
| Concurrent operation | Run phone page, LCD, audio, sensors, lighting, flag, and pumps together | Buttons remain responsive, pump sequence continues, no brownout or unexpected motion. |
