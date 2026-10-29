#include "../firmware/DaetCapitol/ShowLogic.h"
#include <cassert>
#include <cstdint>

int main() {
  ShowLogic show;
  assert(!show.outputs().pump1 && !show.outputs().pump2);

  show.start(100);
  assert(show.outputs().pump1 && !show.outputs().pump2);
  show.update(5099);
  assert(show.outputs().pump1 && !show.outputs().pump2);
  show.update(5100);
  assert(!show.outputs().pump1 && show.outputs().pump2);
  show.update(10100);
  assert(show.outputs().pump1 && !show.outputs().pump2);

  show.stop();
  assert(!show.outputs().pump1 && !show.outputs().pump2);
  show.start(UINT32_MAX - 1000);
  show.update(3999);
  assert(!show.outputs().pump1 && show.outputs().pump2);
  show.stop();

  show.start(0);
  show.update(60000); // A delayed loop resumes with one transition, never rapid relay catch-up.
  assert(!show.outputs().pump1 && show.outputs().pump2);
  show.playbackEnded();
  assert(!show.outputs().pump1 && !show.outputs().pump2);
}
