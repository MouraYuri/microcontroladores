#pragma once
#include <Arduino.h>

void input_init();

// Retorna true UMA vez por aperto do botão (borda de descida, com debounce).
// Segurar o botão não gera novos eventos.
bool input_button_pressed(uint32_t now_ms);
