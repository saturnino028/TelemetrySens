# Sistema de Telemetria Industrial com ESP32-S3 (Heltec V3), Modbus RTU e MQTT

Este repositório contém o código-fonte de um sistema de telemetria IoT, desenvolvido com o framework **ESP-IDF** em **C** e executado nativamente sobre o **FreeRTOS**. O sistema realiza a leitura de sensores de temperatura e umidade em um barramento RS-485 utilizando o protocolo Modbus RTU e publica os dados em um broker MQTT (HiveMQ).

O hardware base desta aplicação é a placa **Heltec WiFi LoRa 32 V3** (baseada no microcontrolador ESP32-S3).

## 🚀 Principais Características

* **Leitura de Sensores Modbus RTU:** Comunicação através de rede RS-485 (Half-Duplex) configurada nos pinos dedicados.
* **Conectividade Wi-Fi e MQTT:** Conexão automatizada e publicação de telemetria em formato JSON.
* **Robustez e Recuperação Automática:** Implementação de Watchdog Timer (WDT), garantindo o reinício do sistema em caso de travamentos, com contabilização de reboots.
* **Sincronização de Tempo (SNTP):** Carimbo de tempo real (timestamp) integrado aos pacotes JSON de telemetria, sincronizado via servidores NTP brasileiros (`https://ntp.br/`).
* **Telemetria Avançada e Diagnósticos:** Além dos dados atmosféricos, envia informações de diagnóstico de hardware como: temperatura interna do chip ESP32-S3, nível de sinal Wi-Fi (RSSI), falhas de leitura Modbus e contador de resets por watchdog.
* **Configuração Remota via MQTT:** Permite alterar o intervalo de envio da telemetria dinamicamente assinando um tópico dedicado.
* **Monitoramento Local Orientado a Eventos:** O botão físico é monitorado por uma task dedicada no FreeRTOS (com controle via Mutex), disparando pacotes imediatos ao ser pressionado.

## 🛠️ Hardware e Pinagem

### Componentes Necessários
- Placa **Heltec WiFi LoRa 32 V3** (ESP32-S3)
- Módulo Transceiver RS-485 (Deve se observar a tensão de alimentação e a necessidade de conversores de nível)
- Sensor de Temperatura/Umidade Modbus (ex: AHT20 via Modbus RTU)

### Mapeamento de Pinos (Pinout)
| Função         | Pino ESP32-S3 | Configuração |
|----------------|--------------|---------|
| Modbus TX      | `GPIO 41`    | Transmissão UART |
| Modbus RX      | `GPIO 42`    | Recepção UART |
| Modbus RTS_DE  | `GPIO 45`    | Controle de Fluxo (Half-Duplex) |
| Botão Externo  | `GPIO 48`    | Entrada (Pull-Down) |

## ⚙️ Como Usar (Instalação e Compilação)

### 1. Pré-requisitos
Certifique-se de ter o **ESP-IDF** instalado e configurado adequadamente em seu ambiente de desenvolvimento. 

### 2. Configurando as Credenciais
Para proteger suas senhas e dados sensíveis, as credenciais não são versionadas no repositório. Você precisa configurar o arquivo de ambiente antes da primeira compilação.

1. No diretório do projeto, localize o arquivo de exemplo `Credenciais_ex.h`.
2. Faça uma cópia deste arquivo e renomeie a cópia para **`Credenciais.h`**.
3. Edite o arquivo `Credenciais.h` preenchendo as informações da sua rede Wi-Fi e do seu broker MQTT:
   ```c
   #define WIFI_SSID "Sua_Rede_WiFi"
   #define WIFI_PASS "Sua_Senha_WiFi"

   #define BROKER_URI "mqtt://seubroker.hivemq.com" 
   #define BROKER_USER "Seu_Usuario"
   #define BROKER_PASS "Sua_Senha_MQTT"
   ```

### 3. Compilando e Gravando no Microcontrolador
Abra o terminal, carregue as variáveis de ambiente do ESP-IDF (ex: `get_idf`) e execute:

```bash
# Configure o target para a arquitetura correta
idf.py set-target esp32s3

# Compile o projeto
idf.py build

# Grave na placa e abra o monitor serial (substitua a porta serial caso necessário)
idf.py flash monitor
```

## 📡 Tópicos MQTT e Payload JSON

O sistema interage com o Broker HiveMQ através de três tópicos principais:

### Publicação (Publish)
* **`telemetry_data`**: Envia os dados de telemetria periodicamente.
  * *Exemplo de Payload Principal:* `{"temperatura_ar": 26.5, "umidade_ar": 58.2, "botao": 0, "temp_chip": 48.1, "erros_modbus": 0, "watchdog_resets": 1, "wifi_rssi": -65, "data_hora": "2026-09-06 14:30:00"}`
  * *Evento de Botão:* `{"evento": "botao_pressionado"}`
* **`metadados`**: Envia diagnósticos críticos ou mudanças de configuração.
  * *Alerta de Watchdog no Boot:* `{"alerta": "Reset by Watchdog", "data_hora": "..."}`
  * *Atualização de Intervalo:* `{"evento": "intervalo_alterado", "novo_tempo_seg": 15, "data_hora": "..."}`

### Assinatura (Subscribe)
* **`telemetry_conf`**: Utilizado para configuração OTA (Over-The-Air) da rotina de operação.
  * *Como usar:* Publique um payload numérico (ex: `15`) representando o novo intervalo de envio em segundos. O limite de segurança implementado aceita valores entre 1 e 3600 segundos.

## 🛡️ Robustez e Tratamento de Falhas

Este firmware foi projetado visando tolerância a falhas em aplicações remotas:
1. **Recuperação Modbus:** Se houver ruído elétrico ou falha na rede RS-485, o sistema ignora a leitura corrompida, incrementa a contagem `contador_erros_modbus` e preserva o laço de execução, tentando novamente na próxima iteração.
2. **Watchdog de Hardware/Software:** O Timeout de WDT está calibrado em 30 segundos. Tasks críticas (como loop principal e monitoramento do botão) enviam resets de confirmação (`esp_task_wdt_reset()`) constantemente. Um deadlock acionará o reinício da placa.
3. **Persistência e Consciência de Estado:** No momento do boot, a função `esp_reset_reason()` avalia se a reinicialização ocorreu de forma anômala (pânico ou watchdog), incrementando o log e publicando imediatamente um alerta de metadados no MQTT.

## 🚀 Próximos Passos (Roadmap)
Visando evoluir a escalabilidade e a robustez do sistema, as seguintes melhorias estão mapeadas para futuras atualizações:

* **Gestão e Resiliência de Dados:** Tratamento de desconexões com retenção/recuperação de mensagens pendentes em uma fila (em memória ou em armazenamento persistente).
* **Provisionamento de Rede (Wi-Fi):** Configuração das credenciais Wi-Fi dinamicamente, sem a necessidade de regravar o firmware inteiro. 