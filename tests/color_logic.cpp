#include "../firmware/DaetCapitol/ColorLogic.h"
#include <cassert>

int main() {
  assert(classifyColor(200, 500, 600) == 1);
  assert(classifyColor(500, 200, 600) == 2);
  assert(classifyColor(600, 500, 200) == 3);
  assert(classifyColor(400, 410, 420) == 0);
  assert(classifyColor(0, 500, 600) == 0);
}
