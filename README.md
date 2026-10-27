# Portaria Eletrônica com ESP32

Portaria eletrônica para casas e condomínios sem portaria física. O entregador
aperta o botão, o morador recebe a notificação no **Telegram** e libera ou
recusa a entrada. O portão fecha sozinho depois de 2 minutos.

Projeto final da cadeira de Microcontroladores. Firmware em C++ (Arduino
framework), desenvolvido no **VSCode + PlatformIO**.

## Como funciona

1. O entregador aperta o botão: toca um som de chamada e o morador recebe uma mensagem no Telegram com os botões **Liberar** / **Recusar**.
2. Enquanto espera a resposta, o **LED amarelo pisca**.
3. **Liberar**: LED verde acende, toca um chime e o relé/RF abre o portão. Após 2 minutos (`GATE_OPEN_DURATION_MS`) o portão fecha automaticamente.
4. **Recusar**: o speaker toca 3 bipes e o portão não é acionado.
5. **Sem resposta** em `RESPONSE_TIMEOUT_MS` (60 s): tratado como recusa (premissa UC04, ainda a confirmar).
6. Sem interação por `IDLE_SLEEP_TIMEOUT_MS`, o ESP32 entra em **deep sleep** e só acorda pelo botão.

O morador também pode digitar `/liberar` ou `/recusar` no chat do bot.

### Estados (`src/main.cpp`)

```
Idle -> Chamando -> Aguardando -> Liberando -> PortaoAberto -> Fechando -> Idle
                              \-> Recusando -> Idle      (recusa, sem resposta ou falha de rede)
Idle (ocioso) -> deep sleep -> botão acorda -> Chamando
```

Se não houver Wi-Fi ou a notificação falhar, o sistema **falha fechado**: sinaliza como recusa e o portão não abre.

## Estrutura de pastas

```
projeto-portaria/
├─ platformio.ini            # envs: esp32dev (placa) e native (testes no PC)
├─ include/
│  └─ config.h.example       # modelo das credenciais e pinos (copie para config.h)
├─ lib/
│  ├─ gate_control/          # abre/fecha o portão (abstrai o relé/RF)
│  ├─ timer_service/         # timer de disparo único (espera do morador e fechamento)
│  ├─ telegram_client/       # Telegram Bot API + interpretação dos comandos
│  └─ power_manager/         # deep sleep e wake-up pelo botão
├─ src/
│  ├─ main.cpp               # inicialização e máquina de estados
│  ├─ board_config.h         # lê config.h, valida e define os padrões
│  ├─ input_manager.*        # botão com debounce
│  ├─ output_manager.*       # LEDs e speaker
│  ├─ wifi_manager.*         # conexão e reconexão
│  └─ relay_driver.h         # pulso no relé/RF (implementa o RelayDriver)
└─ test/
   ├─ test_gate_control/
   ├─ test_timer_service/
   └─ test_telegram_client/
```

A lógica de decisão testável fica em `lib/` (sem depender de hardware); `src/`
tem o código ligado à placa e a máquina de estados.

## Componentes

| Componente | Observação |
| --- | --- |
| ESP32 DevKit | placa alvo (`esp32dev`) |
| Botão | entre o `PIN_BUTTON` e o GND; pino **RTC** (para acordar do deep sleep) |
| LED amarelo e LED verde + resistores | espera / liberação |
| Speaker passivo ou buzzer passivo | os sons são tons gerados por PWM (LEDC). Um buzzer **ativo** só vai emitir um tom fixo |
| Módulo de relé ou RF | pulso para abrir e para fechar o portão |
| Bateria externa | alimentação |

Pinagem padrão (altere em `include/config.h`):

| Função | GPIO |
| --- | --- |
| Botão do entregador | 33 |
| LED amarelo | 25 |
| LED verde | 26 |
| Speaker / buzzer | 27 |
| Relé/RF abrir | 16 |
| Relé/RF fechar | 17 |

Recomendações de montagem:

- Botão com um **resistor de pull-up externo de 10k para 3V3** (além do pull-up interno), para garantir o wake-up do deep sleep.
- Módulo de relé **ativo em nível alto** com pull-down de 10k no pino de entrada. Durante o deep sleep os pinos ficam sem controle; o pull-down garante que o relé não dispare sozinho. Para módulos ativos em nível baixo, ajuste `RELAY_ACTIVE_LEVEL` e use pull-up.
- Se o motor do portão funciona com um único botão de "alterna" (abre/fecha), use o **mesmo pino** em `PIN_RELAY_OPEN` e `PIN_RELAY_CLOSE`.

## Configuração do ambiente

1. Instale o **VSCode** e a extensão **PlatformIO IDE**.
2. Abra a pasta `projeto-portaria` no VSCode. O PlatformIO baixa a plataforma ESP32 e as bibliotecas na primeira abertura.
3. Crie o bot no Telegram com o [@BotFather](https://t.me/BotFather) e anote o **token**.
4. Descubra o seu **chat id** (por exemplo, conversando com o [@userinfobot](https://t.me/userinfobot)). Envie uma mensagem qualquer ao seu bot antes, para o chat existir.
5. Crie o arquivo de credenciais:
   ```bash
   cp include/config.h.example include/config.h
   ```
   e preencha `WIFI_SSID`, `WIFI_PASSWORD`, `TELEGRAM_TOKEN` e `AUTHORIZED_CHAT_ID`. O ESP32 só conecta em redes **2.4 GHz**. O `config.h` está no `.gitignore`: **não o versione**.

## Comandos

```bash
pio run                              # compilar
pio run --target upload              # gravar na placa
pio device monitor                   # monitor serial (115200) com os logs de estado
pio run --target upload && pio device monitor
```

Se a porta serial não for detectada, use `--upload-port` (`/dev/ttyUSB0` no Linux, `COM3` no Windows).

## Testes

```bash
pio test -e native      # lógica pura no computador, sem placa (precisa de g++/clang/MinGW)
pio test -e esp32dev    # os mesmos testes executados na placa conectada
```

Cobertura automática (IDs do Plano de Testes de Software):

| ID | Módulo | Situação |
| --- | --- | --- |
| TU02, TU03 | `gate_control` | coberto |
| TU04, TU05 | `timer_service` | coberto |
| TU06, TU07, TU08 | `telegram_client` (parser e autorização) | coberto |
| TU01, TU09, TU10, TU11, TU12 | botão, Wi-Fi, sleep, sons, LED | dependem de hardware: validar nos testes manuais (TM01, TM03, TM06, TM09-TM11) |

Os fluxos ponta a ponta (TI01-TI06) e os testes de energia são feitos na placa,
conforme o Plano de Testes Manuais de Hardware.

## Decisões de projeto

- **Comandos antigos são descartados.** Como o ESP32 dorme, uma mensagem `/liberar` enviada enquanto ele dormia ficaria na fila do Telegram. Antes de cada notificação o firmware esvazia a fila (`flush_pending`) para que um comando velho nunca abra o portão.
- **Só o chat configurado comanda o portão** (RNF07). Se `AUTHORIZED_CHAT_ID` estiver vazio, ninguém é autorizado.
- **Estado do portão em RAM.** Depois de um reset o portão é assumido como fechado; o firmware só entra em deep sleep depois de fechar o portão.
- **Algumas operações bloqueiam o loop**: sons e pulsos de relé (menos de 1,5 s) e a conexão Wi-Fi (até `WIFI_CONNECT_TIMEOUT_MS`, 15 s). Durante esse tempo o LED amarelo não pisca e o botão não é lido.
- Versões fixadas: `espressif32@^6.9.0` (Arduino-ESP32 2.x) e `ArduinoJson@^6` (a major usada pelo UniversalTelegramBot).

## Status e pontos de atenção

- A lógica pura (`gate_control`, `timer_service`, parser do Telegram) tem testes automatizados.
- O código que fala com a placa (Wi-Fi, Telegram, LEDC, deep sleep) precisa ser **compilado e validado no hardware**: siga o Plano de Testes Manuais de Hardware.
- O comportamento de **timeout sem resposta** é uma premissa (UC04) e está marcado como a confirmar.

## Evoluindo o projeto

Novos componentes (ESP32-CAM, display, leitor NFC) entram como novos módulos em
`lib/`, ligados à máquina de estados de `src/main.cpp` apenas nos pontos de
transição necessários. Toda função de lógica pura nova deve vir com teste em
`test/`. Mantenha os documentos de requisitos, arquitetura e testes em dia.
