#pragma once
// Ponto único de acesso à configuração: inclui include/config.h, confere os
// valores obrigatórios e define os padrões dos opcionais.
#include <Arduino.h>

#if __has_include("config.h")
#include "config.h"
#else
#error "include/config.h nao encontrado. Copie include/config.h.example para include/config.h e preencha os valores."
#endif

// ---- obrigatórios ----
#ifndef WIFI_SSID
#error "Defina WIFI_SSID em include/config.h"
#endif
#ifndef WIFI_PASSWORD
#error "Defina WIFI_PASSWORD em include/config.h"
#endif
#ifndef TELEGRAM_TOKEN
#error "Defina TELEGRAM_TOKEN em include/config.h"
#endif
#ifndef AUTHORIZED_CHAT_ID
#error "Defina AUTHORIZED_CHAT_ID em include/config.h"
#endif

// ---- tempos (padrões) ----
#ifndef GATE_OPEN_DURATION_MS
#define GATE_OPEN_DURATION_MS 120000
#endif
#ifndef IDLE_SLEEP_TIMEOUT_MS
#define IDLE_SLEEP_TIMEOUT_MS 60000
#endif
#ifndef RESPONSE_TIMEOUT_MS
#define RESPONSE_TIMEOUT_MS 60000
#endif
#ifndef WIFI_CONNECT_TIMEOUT_MS
#define WIFI_CONNECT_TIMEOUT_MS 15000
#endif
#ifndef TELEGRAM_POLL_INTERVAL_MS
#define TELEGRAM_POLL_INTERVAL_MS 1000
#endif
#ifndef BUTTON_DEBOUNCE_MS
#define BUTTON_DEBOUNCE_MS 50
#endif

// ---- pinos (padrões) ----
#ifndef PIN_BUTTON
#define PIN_BUTTON 33
#endif
#ifndef PIN_LED_YELLOW
#define PIN_LED_YELLOW 25
#endif
#ifndef PIN_LED_GREEN
#define PIN_LED_GREEN 26
#endif
#ifndef PIN_BUZZER
#define PIN_BUZZER 27
#endif
#ifndef PIN_RELAY_OPEN
#define PIN_RELAY_OPEN 16
#endif
#ifndef PIN_RELAY_CLOSE
#define PIN_RELAY_CLOSE 17
#endif

// ---- relé ----
#ifndef RELAY_ACTIVE_LEVEL
#define RELAY_ACTIVE_LEVEL HIGH
#endif
#ifndef RELAY_PULSE_MS
#define RELAY_PULSE_MS 500
#endif
