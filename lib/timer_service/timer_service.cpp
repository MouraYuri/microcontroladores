#include "timer_service.h"

void OneShotTimer::start(uint32_t now_ms, uint32_t duration_ms) {
  start_ms_ = now_ms;
  duration_ms_ = duration_ms;
  active_ = true;
}

void OneShotTimer::cancel() { active_ = false; }

bool OneShotTimer::update(uint32_t now_ms) {
  if (!active_) return false;
  // A subtração sem sinal funciona mesmo quando millis() dá a volta.
  if (static_cast<uint32_t>(now_ms - start_ms_) >= duration_ms_) {
    active_ = false;
    return true;
  }
  return false;
}
