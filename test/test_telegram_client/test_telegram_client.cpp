// Testes da parte pura do telegram_client (TU06, TU07, TU08 do Plano de Testes).
// A comunicação real com a API do Telegram é validada nos testes manuais (TM02, TM04, TM06).
#include <unity.h>

#include "telegram_client.h"

void setUp(void) {}
void tearDown(void) {}

static int as_int(Command c) { return static_cast<int>(c); }

// TU06
void test_parse_liberar(void) {
  TEST_ASSERT_EQUAL_INT(as_int(Command::Liberar), as_int(parse_command("/liberar")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::Liberar), as_int(parse_command("liberar")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::Liberar), as_int(parse_command("/LIBERAR")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::Liberar), as_int(parse_command("  /Liberar  ")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::Liberar), as_int(parse_command("/liberar@MeuBot")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::Liberar), as_int(parse_command("/liberar agora")));
}

// TU07
void test_parse_recusar(void) {
  TEST_ASSERT_EQUAL_INT(as_int(Command::Recusar), as_int(parse_command("/recusar")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::Recusar), as_int(parse_command("recusar")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::Recusar), as_int(parse_command("/RECUSAR")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::Recusar), as_int(parse_command("/recusar@MeuBot")));
}

void test_parse_ignores_anything_else(void) {
  TEST_ASSERT_EQUAL_INT(as_int(Command::None), as_int(parse_command(nullptr)));
  TEST_ASSERT_EQUAL_INT(as_int(Command::None), as_int(parse_command("")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::None), as_int(parse_command("/")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::None), as_int(parse_command("oi")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::None), as_int(parse_command("/start")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::None), as_int(parse_command("liberarr")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::None), as_int(parse_command("nao liberar")));
  TEST_ASSERT_EQUAL_INT(as_int(Command::None),
                        as_int(parse_command("/liberarliberarliberarliberar")));
}

// TU08 (RNF07)
void test_authorized_chat_is_accepted(void) {
  TEST_ASSERT_TRUE(is_authorized("123456789", "123456789"));
  TEST_ASSERT_TRUE(is_authorized("-100123", "-100123"));  // grupo
}

void test_unauthorized_chat_is_rejected(void) {
  TEST_ASSERT_FALSE(is_authorized("987654321", "123456789"));
  TEST_ASSERT_FALSE(is_authorized("12345678", "123456789"));    // prefixo
  TEST_ASSERT_FALSE(is_authorized("1234567890", "123456789"));  // sufixo extra
}

void test_authorization_fails_closed(void) {
  TEST_ASSERT_FALSE(is_authorized(nullptr, "123"));
  TEST_ASSERT_FALSE(is_authorized("123", nullptr));
  TEST_ASSERT_FALSE(is_authorized("123", ""));  // chat autorizado não configurado
  TEST_ASSERT_FALSE(is_authorized("", ""));
}

static int run_all_tests() {
  UNITY_BEGIN();
  RUN_TEST(test_parse_liberar);
  RUN_TEST(test_parse_recusar);
  RUN_TEST(test_parse_ignores_anything_else);
  RUN_TEST(test_authorized_chat_is_accepted);
  RUN_TEST(test_unauthorized_chat_is_rejected);
  RUN_TEST(test_authorization_fails_closed);
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
