#include "../firmware/DaetCapitol/ButtonLadder.h"
#include <cassert>

int main() {
  assert(decodeButtonMv(5) == 0);
  assert(decodeButtonMv(600) == 1);
  assert(decodeButtonMv(1050) == 2);
  assert(decodeButtonMv(1650) == 3);
  assert(decodeButtonMv(2270) == 4);
  assert(decodeButtonMv(3100) == -1);
}
