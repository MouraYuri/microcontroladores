#include "wifi_manager.h"

#include <WiFi.h>

#include "board_config.h"

bool wifi_is_connected() { return WiFi.status() == WL_CONNECTED; }

bool wifi_connect(uint32_t timeout_ms) {
  if (wifi_is_connected()) return true;

  Serial.printf("[wifi] conectando a %s...\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t start = millis();
  while (!wifi_is_connected() && (millis() - start) < timeout_ms) {
    delay(100);
  }

  if (wifi_is_connected()) {
    Serial.printf("[wifi] conectado, IP %s\n", WiFi.localIP().toString().c_str());
    return true;
  }
  Serial.println("[wifi] falha ao conectar (timeout)");
  return false;
}

bool wifi_ensure_connected(uint32_t timeout_ms) {
  if (wifi_is_connected()) return true;
  WiFi.disconnect();  // descarta a tentativa anterior antes de recomeçar
  return wifi_connect(timeout_ms);
}

void wifi_disconnect() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}
