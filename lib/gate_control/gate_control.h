#pragma once
#include <stdint.h>

// Abstração do hardware que gera o pulso no relé/RF.
// Em produção: ArduinoRelayDriver (src/relay_driver.h). Nos testes: um fake.
class RelayDriver {
 public:
  virtual ~RelayDriver() {}
  virtual void pulse(uint8_t pin, uint32_t duration_ms) = 0;
};

// Controla o portão e evita acionamentos duplicados:
// open() só aciona se o portão estiver fechado, close() só se estiver aberto.
//
// Observação: o estado é mantido em RAM. Após um reset o portão é assumido
// como fechado (o firmware só dorme depois de fechar o portão).
class GateControl {
 public:
  enum class State : uint8_t { Closed, Open };

  GateControl(RelayDriver& driver, uint8_t open_pin, uint8_t close_pin,
              uint32_t pulse_ms);

  bool open();   // true se acionou o relé; false se já estava aberto
  bool close();  // true se acionou o relé; false se já estava fechado
  State state() const { return state_; }

 private:
  RelayDriver& driver_;
  uint8_t open_pin_;
  uint8_t close_pin_;
  uint32_t pulse_ms_;
  State state_;
};
