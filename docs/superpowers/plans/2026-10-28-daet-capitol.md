# Daet Capitol Diorama Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build an ESP32-controlled interactive diorama with sensor lighting, manual controls, alternating pumps, recorded audio, joystick flag movement, status display, and mobile controls.

**Architecture:** An Arduino sketch coordinates independent subsystems using nonblocking updates. All commands update shared system state; a single output layer applies motor, pump, and lighting states. One 12 V battery pack feeds separately protected 12 V and converted 5 V power branches.

**Tech Stack:** Arduino IDE, Arduino C++, Arduino-ESP32; peripheral libraries selected after module identification.

**Spec:** User requirements and clarifications are captured below. This is a draft implementation plan: unconfirmed behavior is explicitly proposed, and hardware-dependent tasks must wait for their listed prerequisites. No firmware has been implemented or hardware electrically verified.

## Global Constraints

- Planned 38-pin NodeMCU ESP-32S v1.1, identified from rear silkscreen. Earlier photographed 30-pin board and expansion board are not assumed to fit the final controller. Exact module/USB interface remain to be confirmed from the front.
- Arduino IDE for uploading and debugging.
- Two 12 V miniature diaphragm pumps; current ratings and repeated-start suitability unconfirmed.
- Power comes from one 12 V battery pack with a step-down converter for a 5 V branch; pack and converter current ratings are unconfirmed.
- Pump 1 runs for 5 seconds, then Pump 2 for 5 seconds; repeat while enabled. Never intentionally energize both together.
- Five dedicated buttons: fountain lighting, streetlights, building lighting, manual/automatic mode, and audio/fountain control.
- Joystick controls flag raising/lowering through a TB6612FNG and yellow geared DC motor; user does not want end-stop switches.
- Color, IR proximity, and sound sensors control corresponding lighting groups in automatic mode.
- LCD reports connection, function status, and button activity; speaker plays recordings.
- Final assembly requires soldered connections and a main power switch.
- At least four four-pin RGB light posts share one color. Two separate one-metre RGB strips have three pins marked 5V, DIN/DO, GND. Exact post wiring/current and strip chipset/LED density, pump currents, motor ratings, and battery/converter capacity remain unconfirmed. Do not assign final pins or purchase drivers from this draft alone.

## Confirmed Inventory and Identification Limits

- Five buttons, joystick, earlier photographed 30-pin ESP32/expansion board, speaker, yellow geared DC motor, TB6612FNG, and two-channel relay module. User plans to use a 38-pin ESP32 for the final build.
- Photo suggests a DFPlayer Mini-style audio module, TCS3200/TCS230-style color sensor, IR obstacle sensor, and microphone module; confirm markings before selecting libraries or wiring. The character LCD's four-pin I2C interface is confirmed by the user.
- Two diaphragm pumps, four-pin RGB light-post lights, and three-pin 5 V addressable-style RGB strips are additional components described by the user.
- LCD I2C supply/pull-up voltage, microSD availability, speaker rating, power converter ratings, protection components, and RGB drivers are not confirmed. End-stop switches are explicitly excluded by the user.

## Proposed Operating Rules — Require Review

- Boot in manual mode with pumps, flag motor, audio, and all lights off. Reset does not resume a show or flag movement.
- Manual/automatic mode affects the three lighting groups only. Joystick and audio/fountain button operate in either mode.
- In manual mode, each lighting button toggles its own group. In automatic mode, those three buttons are ignored and that action is reported.
- On entry to automatic mode, sensors determine lighting states; on entry to manual mode, preserve current light states until changed.
- A valid detected color selects the fountain-light color; no valid color turns that group off. Recognized colors require sensor calibration.
- IR detection keeps streetlights on until 10 seconds after the last detection. Sound above a calibrated threshold keeps building lights on until 10 seconds after the last event. These durations are proposed, adjustable settings.
- Suppress sound activation during onboard audio and for 1 second afterward to prevent self-triggering; manual lighting remains available through manual mode.
- Audio/fountain button starts the recording and pump sequence together, beginning with Pump 1. Another press stops both; recording completion also stops both. A confirmed playback error stops the show and reports the error.
- During a pump transition, command the active pump off before commanding the other on. Determine any required both-off interval from relay release time and pump requirements; do not invent a final timing here.
- Joystick up/down moves the flag only while held; center stops it. There is no position feedback or end-stop switch. A calibrated maximum continuous-run timeout stops the motor if the joystick is held too long. The mechanical design must tolerate end-of-travel without damage.
- Show current mode/connectivity and function statuses on alternating LCD pages; temporarily show the latest button action. Keep a bounded recent-event list in memory and detailed events on Serial Monitor.
- Mobile controls use the same state and command rules as physical controls; no dependency on an internet connection for physical operation. App transport, platform, and allowed commands remain open.

## Review Focus

- Power interruption/reset: pumps and motor remain off through startup, including before firmware initialization.
- Simultaneous commands: mode changes and app/button commands cannot leave conflicting outputs energized.
- Audio feedback: the speaker does not trigger automatic building lighting under the proposed suppression rule.
- Motor overrun: releasing the joystick or reaching the calibrated continuous-run timeout stops the motor; the system does not claim to know the flag's actual position.
- Missing peripherals/network: show a useful fault and keep independent physical controls responsive; no blocking retry loops.

## Planned Files

- `firmware/DaetCapitol/DaetCapitol.ino`: initialize and call subsystem updates.
- `firmware/DaetCapitol/Config.h`: verified pins, active levels, timing, enabled hardware, and calibration.
- `firmware/DaetCapitol/SystemState.h`: mode, lighting, show, flag, connection, and fault state.
- `firmware/DaetCapitol/Controls.h/.cpp`: debounced buttons, joystick interpretation, and shared command handling.
- `firmware/DaetCapitol/Lighting.h/.cpp`: sensor acquisition/calibration and RGB output adapters.
- `firmware/DaetCapitol/Show.h/.cpp`: audio and alternating pump sequence.
- `firmware/DaetCapitol/Flag.h/.cpp`: reversible motor control, joystick dead zone, timeout, and reversal protection.
- `firmware/DaetCapitol/Status.h/.cpp`: LCD pages and bounded activity reporting.
- `firmware/DaetCapitol/Mobile.h/.cpp`: selected transport and app command adapter.
- `docs/hardware.md`: verified inventory, power budget, wiring, and connector schedule.
- `docs/acceptance-tests.md`: bench and integrated test records.
- `tests/control_logic.cpp`: host-side tests of scheduling and command logic without powered loads.

## Task 1: Resolve Hardware and Behavior Prerequisites

- [ ] Review proposed operating rules and decide app platform, connection method, controls, and manual RGB colors.
- [ ] Record pump model/current data, startup demand, permitted duty cycle, repeated-start suitability, and wet/dry operation restrictions in `docs/hardware.md`.
- [ ] Confirm the four-pin light-post common pin, polarity, voltage/current per post, and total count; confirm each one-metre strip's chipset, LEDs per metre, DIN/DO direction, and maximum 5 V current.
- [ ] Confirm the I2C LCD supply/pull-up voltage and address, audio board model/playback-status capability, speaker rating, motor voltage/stall current, and sensor interfaces.
- [ ] Verify relay input compatibility, coil supply, DC motor-load switching suitability, and required suppression/protection. Verify TB6612FNG suitability for the flag motor.
- [ ] Size the battery pack and converter from simultaneous loads, startup demands, desired runtime, and thermal margin. Verify the battery's full/empty voltage range. Specify branch protection, wire sizes, connectors, decoupling, grounding, and USB/backfeed handling.
- [ ] Verify expansion-board power routing rather than assuming its regulator can power all loads. Keep pump currents out of ESP32/header power paths.
- [ ] Verify the direct pin map and five-button ADC1 resistor ladder on the NodeMCU ESP-32S v1.1, preserving USB serial and checking ADC thresholds, sensor voltage compatibility, I2C level shifting, and GPIO5/GPIO12 boot states. The user has no MCP23017; the revised firmware does not use an expander.

**Deliverable/check:** Reviewed behavior and wiring/power tables; no unresolved electrical assumptions before attaching loads. This task determines final driver/library selections and pin assignments.

## Task 2: Firmware State and Physical Controls

**Files:** sketch, `Config`, `SystemState`, `Controls`, and control-logic tests.

**Interfaces:** `beginControls()`, `updateControls(uint32_t nowMs)`; shared `handleCommand(Command command, CommandSource source)` consumes all button/app actions. Define `Command`, `CommandSource`, and `SystemState` here for later tasks.

- [ ] Create pure command/state tests for button debouncing, mode transitions, automatic-mode button handling, conflicting commands, and reset-to-off behavior; confirm they fail before implementing logic.
- [ ] Implement the shared state and nonblocking input handling. Keep configured physical outputs disabled until validated.
- [ ] Compile for the identified board in Arduino IDE and record board package version, settings, and successful result.
- [ ] Check each button and joystick input using Serial Monitor, without pumps/motor attached; rerun control tests successfully.

## Task 3: Lighting and Sensors

**Files:** `Lighting`, relevant `Config` fields, control-logic tests, and acceptance checklist.

**Interfaces:** `beginLighting()`, `updateLighting(uint32_t nowMs)` consumes mode/lighting commands and writes the three lighting groups.

- [ ] Test manual/automatic transitions, detection timeouts, missing/invalid color input, and playback-based sound suppression using synthetic sensor events.
- [ ] Implement the confirmed sensor/RGB adapters and timing rules; calibrate sensor readings under actual diorama illumination.
- [ ] Compile and test each lighting group separately, then all three together; confirm no color-sensor feedback or nuisance sound triggers.

## Task 4: Audio and Alternating Pumps

**Files:** `Show`, relevant `Config` fields, control-logic tests, and acceptance checklist.

**Interfaces:** `beginShow()`, `startShow(uint32_t nowMs)`, `stopShow()`, `updateShow(uint32_t nowMs)`; publishes playback state for sound suppression and status.

- [ ] Test Pump 1 at start and before 5000 ms, transition to Pump 2 at the scheduled boundary, repeated cycles, stop from either phase, timer rollover, and playback end/error. Assert mutual exclusion throughout, including any verified both-off transition interval.
- [ ] Implement scheduling with elapsed-time arithmetic; avoid `delay()` or catch-up bursts of relay switching. Both pump outputs initialize off.
- [ ] Verify relay active levels, default-off hardware behavior, and transition sequence with pumps disconnected, including reset and power-up.
- [ ] Test audio alone, then each pump separately under its specified operating conditions, then the alternating sequence. Verify supply stability and pump/relay temperatures.

## Task 5: Flag Mechanism

**Files:** `Flag`, relevant `Config` fields, control-logic tests, and acceptance checklist.

**Interfaces:** `beginFlag()`, `updateFlag(uint32_t nowMs)` consumes joystick/limit inputs and applies the TB6612FNG direction/enable state.

- [ ] Test centered joystick stop, direction reversal, continuous-run timeout, and reset while moving.
- [ ] Implement motor control with the verified motor supply, hardware default-disable state, joystick dead zone, and driver-appropriate reversal handling.
- [ ] Bench-test unloaded at the correct voltage, then install the flag mechanism; calibrate the protective run timeout and verify that mechanical end-of-travel does not damage the motor or flag.

## Task 6: LCD and Mobile Control

**Files:** `Status`, `Mobile`, and acceptance checklist.

**Interfaces:** `beginStatus()`, `updateStatus(uint32_t nowMs)`, `beginMobile()`, `updateMobile(uint32_t nowMs)`; mobile translates validated requests into Task 2 commands and reads shared state.

- [ ] Implement display pages, latest-button feedback, and fault messages within the confirmed LCD dimensions; avoid continuous clearing/flicker.
- [ ] Implement the selected mobile transport and agreed controls. Define connection status precisely as network link versus active app connection.
- [ ] Verify invalid/duplicate app requests and simultaneous physical commands cannot bypass mode rules or the flag motor timeout. If remote flag movement is included, require a short command lease that stops motion on disconnect.
- [ ] Verify loss of network/app/LCD does not block buttons, timed pump transitions, or flag stop behavior; reconnect reflects actual device state.

## Task 7: Integrated Verification and Soldered Assembly

**Files:** final `docs/hardware.md`, `docs/acceptance-tests.md`, firmware upload instructions.

- [ ] Run the full feature checklist on the prototype, including concurrent playback, pumps, lighting, joystick, LCD, and mobile commands.
- [ ] Exercise reset/power interruption in both pump phases and during flag motion; record output behavior and measured power stability.
- [ ] Perform a supervised endurance run within confirmed component duty limits; record resets, false triggers, heating, leakage, and mechanical binding.
- [ ] Solder only the verified circuit; add labeled connectors, strain relief, protection, and separation between water and electronics. Confirm main-switch behavior for every supply branch and USB programming state.
- [ ] Repeat the acceptance checklist after soldering and preserve the final pin map, library versions, calibration, audio-file instructions, and Arduino upload procedure.

**Completion criterion:** Hardware acceptance checks pass with recorded results. Firmware compilation alone does not establish that the physical system works.
