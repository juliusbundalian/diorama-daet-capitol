#pragma once

#include <stdint.h>

// Hardware-independent fountain timing. The caller applies these outputs to
// verified pump drivers; no GPIO is assigned here.
class ShowLogic {
public:
  struct Outputs {
    bool pump1;
    bool pump2;
  };

  void start(uint32_t nowMs) {
    phaseStartedMs_ = nowMs;
    running_ = true;
    pump1Active_ = true;
  }

  void stop() { running_ = false; }
  void playbackEnded() { stop(); }

  void update(uint32_t nowMs) {
    if (!running_ || static_cast<uint32_t>(nowMs - phaseStartedMs_) < 5000U)
      return;
    pump1Active_ = !pump1Active_;
    phaseStartedMs_ = nowMs;
  }

  Outputs outputs() const {
    if (!running_)
      return {false, false};
    return {pump1Active_, !pump1Active_};
  }

private:
  uint32_t phaseStartedMs_ = 0;
  bool running_ = false;
  bool pump1Active_ = true;
};
