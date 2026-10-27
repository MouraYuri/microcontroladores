#pragma once
#include <stdint.h>

// ---------------------------------------------------------------------------
// Parte pura (sem Arduino): testável no computador com `pio test -e native`.
// ---------------------------------------------------------------------------

enum class Command : uint8_t { None, Liberar, Recusar };

// Interpreta o texto recebido do Telegram. Aceita "/liberar", "liberar",
// "/LIBERAR", "/liberar@NomeDoBot" (e o mesmo para "recusar"); texto extra
// depois da palavra é ignorado. Qualquer outra coisa retorna Command::None.
Command parse_command(const char* text);

// Só o chat configurado pode comandar o portão (RNF07).
// Falha fechado: se o chat autorizado não estiver configurado (nulo/vazio),
// ninguém é autorizado.
bool is_authorized(const char* chat_id, const char* authorized_chat_id);

// ---------------------------------------------------------------------------
// Parte que depende do Arduino / UniversalTelegramBot (só compila na placa).
// ---------------------------------------------------------------------------
#ifdef ARDUINO
#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

class TelegramClient {
 public:
  TelegramClient(const char* token, const char* authorized_chat_id);

  void begin();  // configura o certificado TLS do Telegram

  // Descarta mensagens antigas. Deve ser chamado ANTES de notify_arrival():
  // como o ESP32 dorme, comandos enviados enquanto ele dormia ainda estariam
  // na fila do Telegram e poderiam liberar o portão por engano.
  void flush_pending();

  bool notify_arrival();               // avisa o morador, com botões Liberar/Recusar
  bool send_text(const char* text);    // mensagem simples ao morador
  Command poll();                      // consulta comandos do chat autorizado

 private:
  WiFiClientSecure client_;
  UniversalTelegramBot bot_;
  String chat_id_;
};
#endif  // ARDUINO
