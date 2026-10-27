#pragma once
#include <Arduino.h>

#include "board_config.h"
#include "gate_control.h"

// Implementação do RelayDriver para a placa: gera um pulso no pino do relé/RF.
// Bloqueia durante o pulso (RELAY_PULSE_MS, curto por design).
class ArduinoRelayDriver : public RelayDriver {
 public:
  void begin(uint8_t open_pin, uint8_t close_pin) {
    pinMode(open_pin, OUTPUT);
    pinMode(close_pin, OUTPUT);
    digitalWrite(open_pin, inactive_level());
    digitalWrite(close_pin, inactive_level());
  }

  void pulse(uint8_t pin, uint32_t duration_ms) override {
    digitalWrite(pin, RELAY_ACTIVE_LEVEL);
    delay(duration_ms);
    digitalWrite(pin, inactive_level());
  }

 private:
  static uint8_t inactive_level() {
    return (RELAY_ACTIVE_LEVEL == HIGH) ? LOW : HIGH;
  }
};
