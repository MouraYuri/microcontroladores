#include "output_manager.h"

#include "board_config.h"

static const uint32_t kBlinkIntervalMs = 400;
static const uint8_t kBuzzerChannel = 0;  // usado só no Arduino-ESP32 2.x

static bool blink_enabled = false;
static bool yellow_state = false;
static uint32_t last_toggle_ms = 0;

// ---- buzzer (LEDC) ----
// O Arduino-ESP32 3.x mudou a API do LEDC; o platformio.ini fixa a 2.x
// (espressif32@^6), mas o ramo 3.x fica aqui caso a plataforma seja atualizada.
static void buzzer_init() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(PIN_BUZZER, 2000, 8);
#else
  ledcSetup(kBuzzerChannel, 2000, 8);
  ledcAttachPin(PIN_BUZZER, kBuzzerChannel);
#endif
}

static void buzzer_tone(uint16_t freq_hz) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(PIN_BUZZER, freq_hz);
#else
  ledcWriteTone(kBuzzerChannel, freq_hz);
#endif
}

static void beep(uint16_t freq_hz, uint32_t duration_ms) {
  buzzer_tone(freq_hz);
  delay(duration_ms);
  buzzer_tone(0);
}

// ---- API ----
void output_init() {
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  buzzer_init();
  output_all_off();
}

void output_update(uint32_t now_ms) {
  if (!blink_enabled) return;
  if (static_cast<uint32_t>(now_ms - last_toggle_ms) >= kBlinkIntervalMs) {
    last_toggle_ms = now_ms;
    yellow_state = !yellow_state;
    digitalWrite(PIN_LED_YELLOW, yellow_state ? HIGH : LOW);
  }
}

void output_yellow(bool on) {
  blink_enabled = false;
  yellow_state = on;
  digitalWrite(PIN_LED_YELLOW, on ? HIGH : LOW);
}

void output_yellow_blink(bool enable) {
  blink_enabled = enable;
  if (enable) {
    yellow_state = true;
    last_toggle_ms = millis();
    digitalWrite(PIN_LED_YELLOW, HIGH);
  } else {
    output_yellow(false);
  }
}

void output_green(bool on) { digitalWrite(PIN_LED_GREEN, on ? HIGH : LOW); }

void output_all_off() {
  output_yellow(false);
  output_green(false);
  buzzer_tone(0);
}

void output_play_call() {  // "ding-dong"
  beep(880, 250);
  delay(50);
  beep(660, 400);
}

void output_play_chime() {  // três notas ascendentes
  beep(523, 150);
  beep(659, 150);
  beep(784, 300);
}

void output_play_refusal() {  // 3 bipes graves
  for (int i = 0; i < 3; ++i) {
    beep(300, 250);
    if (i < 2) delay(150);
  }
}
