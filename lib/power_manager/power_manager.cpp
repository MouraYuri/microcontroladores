#include "power_manager.h"

#include <Arduino.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>

bool power_woke_by_button() {
  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0;
}

void power_enter_deep_sleep(uint8_t wake_pin) {
  const gpio_num_t pin = static_cast<gpio_num_t>(wake_pin);

  // Mantém o pull-up do botão ativo durante o deep sleep. Mesmo assim,
  // recomenda-se um resistor de pull-up externo (10k para 3V3) no botão.
  rtc_gpio_pullup_en(pin);
  rtc_gpio_pulldown_dis(pin);

  esp_sleep_enable_ext0_wakeup(pin, 0);  // 0 = acorda em nível baixo
  Serial.flush();
  esp_deep_sleep_start();
}
