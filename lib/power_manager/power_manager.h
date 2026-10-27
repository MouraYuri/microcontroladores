#pragma once
#include <stdint.h>

// true se o ESP32 acordou do deep sleep pelo botão do entregador.
bool power_woke_by_button();

// Entra em deep sleep e acorda quando wake_pin for para nível BAIXO (botão
// pressionado). wake_pin precisa ser um pino RTC (0, 2, 4, 12-15, 25-27,
// 32-39). Esta função não retorna: ao acordar, o programa recomeça em setup().
void power_enter_deep_sleep(uint8_t wake_pin);
