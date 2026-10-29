#include "../firmware/DaetCapitol/ControlLogic.h"
#include <cassert>

int main() {
  ControlLogic state;
  assert(!state.automatic && !state.lights[0] && !state.lights[1] && !state.lights[2]);
  state.pressLight(0);
  assert(state.lights[0]);
  state.toggleMode();
  state.pressLight(0);
  assert(state.lights[0]); // Buttons cannot override sensors in automatic mode.
  state.sensorLight(0, false);
  assert(!state.lights[0]);
  state.toggleMode();
  state.pressLight(0);
  assert(state.lights[0]);
}
