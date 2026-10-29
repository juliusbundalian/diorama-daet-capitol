#pragma once

struct ControlLogic {
  bool automatic = false;
  bool lights[3] = {false, false, false}; // Fountain, street, building.

  void pressLight(unsigned group) {
    if (!automatic && group < 3) lights[group] = !lights[group];
  }
  void sensorLight(unsigned group, bool on) {
    if (automatic && group < 3) lights[group] = on;
  }
  void toggleMode() { automatic = !automatic; }
};
