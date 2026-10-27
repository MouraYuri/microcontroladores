#pragma once
#include <Arduino.h>

// LEDs e speaker. Sem lógica de fluxo: as decisões ficam em main.cpp.

void output_init();
void output_update(uint32_t now_ms);  // chamar a cada volta do loop (pisca o LED amarelo)

void output_yellow(bool on);
void output_yellow_blink(bool enable);
void output_green(bool on);
void output_all_off();

// Os sons bloqueiam durante a reprodução (menos de 1,5 s cada).
void output_play_call();     // chamada do entregador
void output_play_chime();    // confirmação de liberação
void output_play_refusal();  // recusa: 3 bipes
