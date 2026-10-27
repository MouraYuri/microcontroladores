#include "telegram_client.h"

#include <ctype.h>
#include <string.h>

Command parse_command(const char* text) {
  if (text == nullptr) return Command::None;

  while (isspace(static_cast<unsigned char>(*text))) ++text;
  if (*text == '/') ++text;

  char word[16];
  size_t n = 0;
  while (*text != '\0' && !isspace(static_cast<unsigned char>(*text)) &&
         *text != '@') {
    if (n >= sizeof(word) - 1) return Command::None;  // palavra longa demais
    word[n++] = static_cast<char>(tolower(static_cast<unsigned char>(*text)));
    ++text;
  }
  word[n] = '\0';

  if (strcmp(word, "liberar") == 0) return Command::Liberar;
  if (strcmp(word, "recusar") == 0) return Command::Recusar;
  return Command::None;
}

bool is_authorized(const char* chat_id, const char* authorized_chat_id) {
  if (chat_id == nullptr || authorized_chat_id == nullptr) return false;
  if (authorized_chat_id[0] == '\0') return false;
  return strcmp(chat_id, authorized_chat_id) == 0;
}

#ifdef ARDUINO

static const char kInlineKeyboard[] =
    "[[{\"text\":\"✅ Liberar\",\"callback_data\":\"/liberar\"},"
    "{\"text\":\"❌ Recusar\",\"callback_data\":\"/recusar\"}]]";

TelegramClient::TelegramClient(const char* token, const char* authorized_chat_id)
    : bot_(token, client_), chat_id_(authorized_chat_id) {}

void TelegramClient::begin() { client_.setCACert(TELEGRAM_CERTIFICATE_ROOT); }

void TelegramClient::flush_pending() {
  // Limite de voltas para nunca travar caso a rede esteja instável.
  for (int i = 0; i < 10; ++i) {
    int n = bot_.getUpdates(bot_.last_message_received + 1);
    if (n <= 0) break;
  }
}

bool TelegramClient::notify_arrival() {
  return bot_.sendMessageWithInlineKeyboard(
      chat_id_, "🔔 Há um entregador no portão. Deseja liberar a entrada?", "",
      kInlineKeyboard);
}

bool TelegramClient::send_text(const char* text) {
  return bot_.sendMessage(chat_id_, text, "");
}

Command TelegramClient::poll() {
  Command result = Command::None;

  int n = bot_.getUpdates(bot_.last_message_received + 1);
  while (n > 0) {
    for (int i = 0; i < n; ++i) {
      const telegramMessage& m = bot_.messages[i];

      if (!is_authorized(m.chat_id.c_str(), chat_id_.c_str())) {
        Serial.println("[telegram] mensagem ignorada: chat nao autorizado");
        continue;
      }
      if (m.type == "callback_query") {
        bot_.answerCallbackQuery(m.query_id);  // remove o "carregando" do botão
      }

      Command c = parse_command(m.text.c_str());
      if (c != Command::None && result == Command::None) result = c;
    }
    n = bot_.getUpdates(bot_.last_message_received + 1);
  }
  return result;
}

#endif  // ARDUINO
