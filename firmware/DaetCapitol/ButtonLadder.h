#pragma once

// 10k pull-up to 3.3V; buttons to ground via 0, 2.2k, 4.7k, 10k, 22k.
// Values are approximate millivolts; record actual readings and calibrate.
inline int decodeButtonMv(int mv) {
  if (mv < 300) return 0;
  if (mv < 825) return 1;
  if (mv < 1350) return 2;
  if (mv < 1960) return 3;
  if (mv < 2800) return 4;
  return -1;
}
