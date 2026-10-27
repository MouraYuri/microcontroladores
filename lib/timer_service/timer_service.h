#pragma once
#include <stdint.h>

// Temporizador de disparo único, sem bloqueio, baseado em millis().
// Usado para: tempo de espera da resposta do cliente e fechamento do portão.
//
// Seguro contra o estouro (wrap-around) do millis() a cada ~49 dias.
class OneShotTimer {
 public:
  void start(uint32_t now_ms, uint32_t duration_ms);
  void cancel();
  bool active() const { return active_; }

  // Chamar a cada volta do loop. Retorna true UMA única vez, quando o tempo
  // acaba. Um timer cancelado ou já disparado retorna sempre false.
  bool update(uint32_t now_ms);

 private:
  uint32_t start_ms_ = 0;
  uint32_t duration_ms_ = 0;
  bool active_ = false;
};
