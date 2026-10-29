#pragma once
#include <stdint.h>

// Returns 0 for no confident color, 1 red, 2 blue, 3 green.
// Pulse limits and dominance must be calibrated against the actual sensor.
inline uint8_t classifyColor(uint32_t red, uint32_t blue, uint32_t green,
                             uint32_t maxPeriodUs = 1200, uint8_t dominancePercent = 80) {
  const uint32_t periods[3] = {red, blue, green};
  for (uint8_t i = 0; i < 3; ++i) if (!periods[i]) return 0;
  uint8_t best = 0, second = 1;
  if (periods[second] < periods[best]) { best = 1; second = 0; }
  if (periods[2] < periods[best]) { second = best; best = 2; }
  else if (periods[2] < periods[second]) second = 2;
  if (periods[best] > maxPeriodUs || periods[best] * 100U > periods[second] * dominancePercent) return 0;
  return best + 1;
}
