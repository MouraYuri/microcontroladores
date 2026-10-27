#include "input_manager.h"

#include "board_config.h"

// Botão ligado entre o pino e GND: pressionado = LOW.
static bool last_raw = false;
static bool stable_pressed = false;
static uint32_t last_change_ms = 0;

void input_init() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  // Começa com o estado atual: se o botão ainda estiver apertado após acordar
  // do deep sleep, isso não vira um segundo "aperto" (o wake-up já é tratado
  // em main.cpp).
  last_raw = (digitalRead(PIN_BUTTON) == LOW);
  stable_pressed = last_raw;
  last_change_ms = millis();
}

bool input_button_pressed(uint32_t now_ms) {
  const bool raw = (digitalRead(PIN_BUTTON) == LOW);

  if (raw != last_raw) {
    last_raw = raw;
    last_change_ms = now_ms;
  }

  if (raw != stable_pressed &&
      static_cast<uint32_t>(now_ms - last_change_ms) >= BUTTON_DEBOUNCE_MS) {
    stable_pressed = raw;
    if (stable_pressed) return true;
  }
  return false;
}
