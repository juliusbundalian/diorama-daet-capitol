# Daet Capitol Diorama hardware record

## Identified so far

- Planned controller: 38-pin NodeMCU ESP-32S v1.1 (identified from the rear silkscreen photo, `IMG_20261029_111013.jpg`). The earlier photographed 30-pin ESP32 and its expansion board are no longer the assumed final controller; the old expansion board is not assumed compatible.
- Five push buttons; joystick; geared DC motor; TB6612FNG motor driver.
- Two 12 V miniature diaphragm pumps; two-channel relay module.
- One 12 V battery pack, with a step-down converter supplying a 5 V branch (user-selected power architecture).
- Speaker and probable DFPlayer Mini-style microSD audio module.
- Character LCD with confirmed four-pin I2C interface (GND, VCC, SDA, SCL); IR proximity module, microphone module, and TCS3200-style color sensor remain photo-based identifications.
- At least four four-pin RGB light-post lights, all displaying the same color at the same time (likely common-pin R/G/B; common-anode versus common-cathode and voltage unconfirmed).
- Two separate one-metre, three-pin 5 V RGB strips marked 5V, DIN/DO, GND (likely addressable pixels; exact chipset, LEDs per metre, and data direction unconfirmed).

The photo-based identifications need checking against markings on the actual modules before wiring.

## Measurements and decisions needed for final wiring

| Item | Needed detail | Why |
| --- | --- | --- |
| Each pump | Model, running and startup current, allowed cycling and duty cycle | Size supply and verify switching method. |
| RGB light posts | Common-anode versus common-cathode, voltage, built-in resistors, and current per color | Choose current-limiting and three-color drive circuits for four or more posts together. |
| Two 1 m 5 V RGB strips | Chipset, LEDs per metre, maximum current, and DIN/DO direction | Size 5 V converter and select data driver/library for two independent groups. |
| 12 V battery pack | Chemistry, capacity, full/empty voltage range, maximum continuous and peak output current, connector | Check runtime, startup demand, and whether actual battery voltage stays within every load's limits. |
| 5 V step-down converter | Input range, continuous/peak output current, and output voltage under load | Check ESP32/audio/LCD supply stability. |
| Flag motor | Voltage and stall current | Check TB6612FNG and motor supply. |
| Flag travel | Measured time to move from bottom to top, gear/drum arrangement, and acceptable maximum motor run time | Set a protective software timeout; joystick release stops motion, with no position feedback. |
| Audio module | Exact model, playback status output, speaker rating | Select library and detect recording completion. |
| I2C LCD | Supply voltage and I2C pull-up voltage; address is confirmed as 0x27 | Choose safe interface voltage and library configuration. |
| Expansion board | Power schematic or verified regulator/output labels | Avoid routing pump current through signal headers. |
| 38-pin ESP32 board | Front/module marking and USB interface | Confirm the exact Arduino IDE board selection and any module-specific restrictions. |

## Current power concept

One 12 V battery pack feeds the two pumps through individually verified switching channels. A step-down converter makes a 5 V branch for components whose confirmed ratings call for 5 V; the ESP32 is supplied only through a verified 5 V input path on its board or expansion board. Components that require 3.3 V use a verified 3.3 V rail and compatible signal levels. Lighting gets a branch matched to its actual rating. Signal grounds must share a reference where the interfaces require it. The main switch and branch protection must cover both the 12 V and converted branches. All branch sizing, battery runtime, and protection remain open until the ratings above are known.

The current Arduino sketch is a serial bench simulation: it has no GPIO outputs, so uploading it cannot switch pumps. Final pin assignments and electrical hookup follow verification of the 38-pin board and the other items above. A 38-pin header does not imply 38 usable GPIOs.

The two 5 V addressable strips need a 5 V power branch sized for their combined pixel count and reliable 3.3 V-to-5 V data interfaces. Their DIN pads are inputs from the controller; DO pads are for chaining. The provisional design uses one data signal per strip so fountain and building colors can differ. The four-pin light posts are a separate RGB circuit: likely one shared pin plus individual red, green, and blue channels. All four posts can share three control signals while each LED still has appropriate current limiting. Their channel drivers depend on total current and polarity. These two RGB types must not be wired as though they have the same interface.

## Controller pin labels visible in the rear photo

The left row includes P23, P22, TX, RX, P21, P19, P18, P5, P17, P16, P4, P0, P2, P15, SD1, SD0, and CLK. The right row includes SVP, SVN, P34, P35, P32, P33, P25, P26, P27, P14, P12, P13, SD2, and SD3. Remaining header positions are power, ground, or EN. Pin labels beginning with P denote GPIO numbers; SVP/SVN are GPIO36/39.

Reserve SD0–SD3 and CLK for onboard flash. Keep TX/RX available for USB serial debugging. GPIO34, GPIO35, SVP, and SVN are input only and lack internal pull resistors. GPIO0, GPIO2, GPIO5, GPIO12, and GPIO15 affect boot behavior, so allocate them only after checking attached circuits' startup states. The direct pin map uses GPIO5 for a TCS3200 selection input and GPIO12 for the blue streetlight driver; verify their attached circuits preserve boot states. The confirmed I2C LCD uses P21/P22; check its pull-up voltage before connecting to ESP32.

### Provisional signal count

Five buttons share one ADC1 input through a resistor ladder (1), the joystick uses one axis (1), IR (1), sound (1), color sensor with fixed frequency scaling (3), flag driver (3), pump controls (2), audio module (2), I2C LCD (2), one shared RGB light-post group (3), and two independent addressable RGB strip data lines (2): **21 ESP32 signal connections**. This avoids a GPIO expander while retaining USB serial. Simultaneous physical button presses are unsupported; resistor values and ADC thresholds require bench calibration. The user does not own an MCP23017.

The user chose joystick-only flag control and no limit switches. Releasing the joystick stops the motor. A calibrated maximum continuous run time is a backup against holding the joystick too long, but cannot prove where the flag is. The mechanical design must tolerate reaching either end without damaging the motor or flag.
