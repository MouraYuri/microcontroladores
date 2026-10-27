#include "gate_control.h"

GateControl::GateControl(RelayDriver& driver, uint8_t open_pin,
                         uint8_t close_pin, uint32_t pulse_ms)
    : driver_(driver),
      open_pin_(open_pin),
      close_pin_(close_pin),
      pulse_ms_(pulse_ms),
      state_(State::Closed) {}

bool GateControl::open() {
  if (state_ == State::Open) return false;
  driver_.pulse(open_pin_, pulse_ms_);
  state_ = State::Open;
  return true;
}

bool GateControl::close() {
  if (state_ == State::Closed) return false;
  driver_.pulse(close_pin_, pulse_ms_);
  state_ = State::Closed;
  return true;
}
