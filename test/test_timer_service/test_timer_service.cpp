// Testes do módulo timer_service (TU04, TU05 do Plano de Testes de Software).
#include <unity.h>

#include "timer_service.h"

void setUp(void) {}
void tearDown(void) {}

// TU04: temporizador de 2 minutos dispara exatamente após 120 s
void test_fires_exactly_after_duration(void) {
  OneShotTimer timer;
  timer.start(1000, 120000);

  TEST_ASSERT_FALSE(timer.update(1000));
  TEST_ASSERT_FALSE(timer.update(120999));  // 1 ms antes
  TEST_ASSERT_TRUE(timer.update(121000));   // exatamente 120 s depois
}

void test_fires_only_once(void) {
  OneShotTimer timer;
  timer.start(0, 100);

  TEST_ASSERT_TRUE(timer.update(100));
  TEST_ASSERT_FALSE(timer.update(101));
  TEST_ASSERT_FALSE(timer.update(100000));
  TEST_ASSERT_FALSE(timer.active());
}

// TU05: timer cancelado não dispara
void test_cancelled_timer_does_not_fire(void) {
  OneShotTimer timer;
  timer.start(0, 100);
  timer.cancel();

  TEST_ASSERT_FALSE(timer.active());
  TEST_ASSERT_FALSE(timer.update(100));
  TEST_ASSERT_FALSE(timer.update(100000));
}

void test_timer_never_started_does_not_fire(void) {
  OneShotTimer timer;
  TEST_ASSERT_FALSE(timer.update(123456));
}

void test_can_be_restarted_after_firing(void) {
  OneShotTimer timer;
  timer.start(0, 100);
  TEST_ASSERT_TRUE(timer.update(100));

  timer.start(500, 100);
  TEST_ASSERT_FALSE(timer.update(599));
  TEST_ASSERT_TRUE(timer.update(600));
}

// millis() dá a volta a cada ~49 dias: o timer deve continuar correto
void test_works_across_millis_wraparound(void) {
  OneShotTimer timer;
  timer.start(0xFFFFFF00u, 0x200);  // termina em 0x00000100 após a volta

  TEST_ASSERT_FALSE(timer.update(0xFFFFFFFFu));
  TEST_ASSERT_FALSE(timer.update(0x000000FFu));
  TEST_ASSERT_TRUE(timer.update(0x00000100u));
}

static int run_all_tests() {
  UNITY_BEGIN();
  RUN_TEST(test_fires_exactly_after_duration);
  RUN_TEST(test_fires_only_once);
  RUN_TEST(test_cancelled_timer_does_not_fire);
  RUN_TEST(test_timer_never_started_does_not_fire);
  RUN_TEST(test_can_be_restarted_after_firing);
  RUN_TEST(test_works_across_millis_wraparound);
  return UNITY_END();
}

#ifdef ARDUINO
#include <Arduino.h>
void setup() {
  delay(2000);
  run_all_tests();
}
void loop() {}
#else
int main(int, char**) { return run_all_tests(); }
#endif
