#pragma once
#include <Arduino.h>

bool wifi_is_connected();
bool wifi_connect(uint32_t timeout_ms);            // conecta (ou confirma que já está conectado)
bool wifi_ensure_connected(uint32_t timeout_ms);   // reconecta se a conexão caiu
void wifi_disconnect();                            // desliga o rádio (antes do deep sleep)
