// Testes do módulo gate_control (TU02, TU03 do Plano de Testes de Software).
// Rodam no PC:     pio test -e native
// Rodam na placa:  pio test -e esp32dev
#include <unity.h>

#include "gate_control.h"

static const uint8_t kOpenPin = 16;
static const uint8_t kClosePin = 17;
static const uint32_t kPulseMs = 500;

// RelayDriver falso: só registra os pulsos pedidos.
struct FakeRelay : RelayDriver {
  uint8_t pins[8] = {0};
  uint32_t durations[8] = {0};
  int count = 0;

  void pulse(uint8_t pin, uint32_t duration_ms) override {
    if (count < 8) {
      pins[count] = pin;
      durations[count] = duration_ms;
    }
    ++count;
  }
};

void setUp(void) {}
void tearDown(void) {}

// TU02: comando de abertura aciona o relé uma única vez
void test_open_pulses_open_pin_once(void) {
  FakeRelay relay;
  GateControl gate(relay, kOpenPin, kClosePin, kPulseMs);

  TEST_ASSERT_TRUE(gate.open());
  TEST_ASSERT_EQUAL_INT(1, relay.count);
  TEST_ASSERT_EQUAL_UINT8(kOpenPin, relay.pins[0]);
  TEST_ASSERT_EQUAL_UINT32(kPulseMs, relay.durations[0]);
  TEST_ASSERT_TRUE(gate.state() == GateControl::State::Open);

  // segundo comando de abertura não deve acionar de novo
  TEST_ASSERT_FALSE(gate.open());
  TEST_ASSERT_EQUAL_INT(1, relay.count);
}

// TU03: comando de fechamento aciona o relé de fechar, sem reabrir
void test_close_pulses_close_pin_without_reopening(void) {
  FakeRelay relay;
  GateControl gate(relay, kOpenPin, kClosePin, kPulseMs);

  gate.open();
  TEST_ASSERT_TRUE(gate.close());
  TEST_ASSERT_EQUAL_INT(2, relay.count);
  TEST_ASSERT_EQUAL_UINT8(kClosePin, relay.pins[1]);
  TEST_ASSERT_TRUE(gate.state() == GateControl::State::Closed);

  TEST_ASSERT_FALSE(gate.close());
  TEST_ASSERT_EQUAL_INT(2, relay.count);
}

// Portão fechado não recebe pulso de fechamento (nem a recusa o aciona: RN03)
void test_close_when_already_closed_does_nothing(void) {
  FakeRelay relay;
  GateControl gate(relay, kOpenPin, kClosePin, kPulseMs);

  TEST_ASSERT_FALSE(gate.close());
  TEST_ASSERT_EQUAL_INT(0, relay.count);
  TEST_ASSERT_TRUE(gate.state() == GateControl::State::Closed);
}

// Motores "toggle": o mesmo pino abre e fecha
void test_same_pin_for_open_and_close(void) {
  FakeRelay relay;
  GateControl gate(relay, kOpenPin, kOpenPin, kPulseMs);

  gate.open();
  gate.close();
  TEST_ASSERT_EQUAL_INT(2, relay.count);
  TEST_ASSERT_EQUAL_UINT8(kOpenPin, relay.pins[0]);
  TEST_ASSERT_EQUAL_UINT8(kOpenPin, relay.pins[1]);
}

static int run_all_tests() {
  UNITY_BEGIN();
  RUN_TEST(test_open_pulses_open_pin_once);
  RUN_TEST(test_close_pulses_close_pin_without_reopening);
  RUN_TEST(test_close_when_already_closed_does_nothing);
  RUN_TEST(test_same_pin_for_open_and_close);
  return UNITY_END();
}

#ifdef ARDUINO
#include <Arduino.h>
void setup() {
  delay(2000);  // dá tempo do monitor serial conectar
  run_all_tests();
}
void loop() {}
#else
int main(int, char**) { return run_all_tests(); }
#endif
