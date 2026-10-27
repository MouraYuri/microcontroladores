// Portaria Eletrônica com ESP32 - inicialização e máquina de estados.
//
// Fluxo (ver Projeto de Arquitetura):
//   Idle -> Chamando -> Aguardando -> Liberando -> PortaoAberto -> Fechando -> Idle
//                                  \-> Recusando -> Idle   (recusa ou sem resposta)
// Em Idle, sem interação por IDLE_SLEEP_TIMEOUT_MS, o ESP32 entra em deep sleep
// e só acorda pelo botão do entregador (ao acordar, setup() roda de novo).

#include <Arduino.h>

#include "board_config.h"
#include "gate_control.h"
#include "input_manager.h"
#include "output_manager.h"
#include "power_manager.h"
#include "relay_driver.h"
#include "telegram_client.h"
#include "timer_service.h"
#include "wifi_manager.h"

enum class State : uint8_t {
  Idle,
  Chamando,      // transitório: som, Wi-Fi, notificação
  Aguardando,    // espera a resposta do morador
  Liberando,     // transitório: abre o portão
  PortaoAberto,  // espera o fechamento automático
  Fechando,      // transitório: fecha o portão
  Recusando      // transitório: sinaliza recusa
};

static ArduinoRelayDriver relay;
static GateControl gate(relay, PIN_RELAY_OPEN, PIN_RELAY_CLOSE, RELAY_PULSE_MS);
static TelegramClient telegram(TELEGRAM_TOKEN, AUTHORIZED_CHAT_ID);
static OneShotTimer response_timer;  // tempo máximo de espera pelo morador
static OneShotTimer gate_timer;      // fechamento automático do portão

static State state = State::Idle;
static uint32_t idle_since_ms = 0;
static uint32_t last_poll_ms = 0;
static const char* refusal_message = nullptr;  // aviso enviado ao morador em Recusando

static const char* state_name(State s) {
  switch (s) {
    case State::Idle:         return "Idle";
    case State::Chamando:     return "Chamando";
    case State::Aguardando:   return "Aguardando";
    case State::Liberando:    return "Liberando";
    case State::PortaoAberto: return "PortaoAberto";
    case State::Fechando:     return "Fechando";
    case State::Recusando:    return "Recusando";
  }
  return "?";
}

static void go(State next) {
  Serial.printf("[estado] %s -> %s\n", state_name(state), state_name(next));
  state = next;
  if (next == State::Idle) idle_since_ms = millis();
}

static void enter_deep_sleep() {
  Serial.println("[sleep] entrando em deep sleep");
  output_all_off();
  wifi_disconnect();
  power_enter_deep_sleep(PIN_BUTTON);  // não retorna
}

// ---- estados transitórios ----

static void handle_chamando() {
  output_yellow(true);  // aceso fixo enquanto conecta; pisca ao aguardar
  output_play_call();

  if (!wifi_ensure_connected(WIFI_CONNECT_TIMEOUT_MS)) {
    Serial.println("[erro] sem Wi-Fi: portao permanece fechado");
    refusal_message = nullptr;
    go(State::Recusando);
    return;
  }

  telegram.flush_pending();  // ignora comandos antigos na fila do Telegram

  bool sent = false;
  for (int attempt = 0; attempt < 3 && !sent; ++attempt) {
    sent = telegram.notify_arrival();
    if (!sent) delay(1000);
  }
  if (!sent) {
    Serial.println("[erro] falha ao notificar o morador: portao permanece fechado");
    refusal_message = nullptr;
    go(State::Recusando);
    return;
  }

  output_yellow_blink(true);
  response_timer.start(millis(), RESPONSE_TIMEOUT_MS);
  last_poll_ms = millis();
  go(State::Aguardando);
}

static void handle_liberando() {
  output_yellow_blink(false);
  output_green(true);

  gate.open();  // abre primeiro: é o que o entregador está esperando
  gate_timer.start(millis(), GATE_OPEN_DURATION_MS);

  output_play_chime();
  telegram.send_text("✅ Portão liberado. Fecha automaticamente em 2 minutos.");
  go(State::PortaoAberto);
}

static void handle_fechando() {
  gate.close();
  output_green(false);
  telegram.send_text("🔒 Portão fechado.");
  go(State::Idle);
}

static void handle_recusando() {
  output_yellow_blink(false);
  output_play_refusal();
  if (refusal_message != nullptr) {
    telegram.send_text(refusal_message);
    refusal_message = nullptr;
  }
  go(State::Idle);
}

// ---- estados de espera ----

static void handle_aguardando(uint32_t now) {
  if (response_timer.update(now)) {
    // Premissa UC04 (a confirmar): sem resposta, o portão permanece fechado.
    refusal_message = "⌛ Sem resposta: a entrada não foi liberada.";
    go(State::Recusando);
    return;
  }

  if (static_cast<uint32_t>(now - last_poll_ms) < TELEGRAM_POLL_INTERVAL_MS) return;
  last_poll_ms = now;

  if (!wifi_is_connected()) {
    wifi_ensure_connected(WIFI_CONNECT_TIMEOUT_MS);  // reconexão (RF13)
    return;
  }

  switch (telegram.poll()) {
    case Command::Liberar:
      go(State::Liberando);
      break;
    case Command::Recusar:
      response_timer.cancel();
      refusal_message = "Acesso recusado.";
      go(State::Recusando);
      break;
    case Command::None:
      break;
  }
}

// ---- Arduino ----

void setup() {
  Serial.begin(115200);
  delay(100);

  const bool woke_by_button = power_woke_by_button();
  Serial.println(woke_by_button ? "[boot] acordou pelo botao"
                                : "[boot] inicializacao normal");

  output_init();
  relay.begin(PIN_RELAY_OPEN, PIN_RELAY_CLOSE);
  input_init();
  telegram.begin();

  idle_since_ms = millis();
  if (woke_by_button) {
    go(State::Chamando);  // o aperto que acordou o ESP32 já é a chamada
  }
}

void loop() {
  const uint32_t now = millis();
  output_update(now);

  switch (state) {
    case State::Idle:
      if (input_button_pressed(now)) {
        go(State::Chamando);
      } else if (static_cast<uint32_t>(now - idle_since_ms) >= IDLE_SLEEP_TIMEOUT_MS) {
        enter_deep_sleep();
      }
      break;

    case State::Chamando:     handle_chamando(); break;
    case State::Aguardando:   handle_aguardando(now); break;
    case State::Liberando:    handle_liberando(); break;
    case State::PortaoAberto:
      if (gate_timer.update(now)) go(State::Fechando);
      break;
    case State::Fechando:     handle_fechando(); break;
    case State::Recusando:    handle_recusando(); break;
  }
}
