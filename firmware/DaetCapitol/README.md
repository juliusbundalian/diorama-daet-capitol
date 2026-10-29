# Daet Capitol firmware draft

Open `DaetCapitol.ino` with Arduino IDE using ESP32 board package 3.3.2 or compatible. The code uses only libraries bundled with Arduino-ESP32: `Wire`, `WiFi`, and `WebServer`. The LCD backpack is driven directly at address `0x27`. Serial Monitor runs at 115200 baud. Use a conservative Upload Speed of 115200 if high-speed uploads fail.

This is a configurable implementation for the currently described hardware. It has compiled on the local Arduino ESP32 core, but **no actual connected device or pump has been tested by the assistant**. Confirm every electrical requirement before changing the three `ENABLE_*` settings in `Config.h`. By default, pumps, motor, and RGB outputs are disabled.

## Provisional pin map

| Function | ESP32 / expander connection |
| --- | --- |
| I2C SDA/SCL | GPIO21 / GPIO22 for LCD 0x27 |
| Five buttons | One resistor ladder to GPIO39, in order: fountain light, streetlight, building light, mode, show; one button at a time |
| IR / sound digital outputs | GPIO35 / GPIO26, active low assumed |
| TCS3200 output / select | OUT GPIO36; S2 GPIO5, S3 GPIO25; S0/S1 and OE require verified fixed connections |
| Joystick Y | GPIO34; firmware uses only this axis for flag motion |
| TB6612FNG | AIN1 GPIO32, AIN2 GPIO33, PWMA GPIO23; STBY and motor supply require verified wiring |
| Pump relay inputs | GPIO18 / GPIO19; assumed active low, outputs disabled by default |
| Audio module | ESP32 RX GPIO16, TX GPIO17; DFPlayer-style 9600 baud serial; first track numbered 1 |
| Four shared-color streetlights | Red GPIO13, green GPIO14, blue GPIO12 through verified current drivers and individual LED current limiting |
| Fountain / building addressable strips | GPIO4 / GPIO27 through 3.3 V-to-5 V data buffers; separate 5 V power branch |

Do not connect pumps, motor, RGB LED loads, or a 5 V I2C bus directly to ESP32 GPIO. Confirm whether the LCD's backpack pulls SDA/SCL to 5 V; add a bidirectional I2C level shifter if needed. Confirm relay coil supply and input polarity before enabling pump outputs. GPIO5 and GPIO12 affect ESP32 boot behavior: the TCS input and streetlight driver must not force them to the wrong level at startup. A physical main switch and branch protection are outside firmware.

### Five-button resistor ladder

Connect GPIO39 to 3.3 V through a **10 kΩ pull-up resistor**. Connect each button between GPIO39 and ground through its own resistor: fountain **0 Ω** (direct switch), streetlight **2.2 kΩ**, building **4.7 kΩ**, mode **10 kΩ**, show **22 kΩ**. Ground and 3.3 V must come from the ESP32 logic supply. Do not connect this circuit to 5 V. Simultaneous button presses are not supported. The firmware prints the measured button millivolts; adjust `ButtonLadder.h` thresholds if actual readings differ. No MCP23017 is needed.

## Behavior and current assumptions

- Boots in manual mode with lights off, show off, and motor stopped.
- In manual mode, three buttons or phone buttons toggle the three lighting groups. Mode button switches to automatic mode, where sensors own lighting states.
- Automatic streetlights stay on for 10 seconds after IR detection. Building lights stay on for 10 seconds after a sound event; onboard playback suppresses this sound trigger.
- The color sensor selects fountain strip red, blue, or green by relative pulse periods. It requires the winning color to be at least 20% stronger than the next channel and its pulse period to be no more than 1200 microseconds. These provisional values and the filter mapping need calibration.
- Show button starts DFPlayer track 1 and alternating pumps, five seconds per pump, with a 100 ms both-off relay interval. Another press or a DFPlayer finished event stops them.
- Joystick up/down moves the flag; center stops it. Continuous movement stops after `FLAG_MAX_RUN_MS`. No end switches or position tracking exist.
- LCD alternates between statuses and the latest control action. The phone page shows state and five buttons.
- The ESP32 starts a local WiFi access point named `DaetCapitol` with password `DaetCapitol2026`; connect a phone and open `http://192.168.4.1` in its browser. This is a provisional phone interface, pending the user's app choice.
- The strip driver assumes WS2812-compatible timing, GRB color order, and 30 pixels per strip. Change the count and driver after identifying the actual strip chipset and pixel density.

## Host logic tests

From the repository root:

```sh
c++ -std=c++17 -Wall -Wextra -Werror tests/show_logic.cpp -o /tmp/show_logic_test
/tmp/show_logic_test
c++ -std=c++17 -Wall -Wextra -Werror tests/control_logic.cpp -o /tmp/control_logic_test
/tmp/control_logic_test
c++ -std=c++17 -Wall -Wextra -Werror tests/color_logic.cpp -o /tmp/color_logic_test
/tmp/color_logic_test
c++ -std=c++17 -Wall -Wextra -Werror tests/button_ladder.cpp -o /tmp/button_ladder_test
/tmp/button_ladder_test
```

Use `docs/component-tests.md` for the physical acceptance checklist.
