#pragma once

#include "esp_err.h"

/**
 * @brief Inicializa a interface de rede, o NVS e conecta ao Wi-Fi.
 * A função bloqueia até que o IP seja obtido ou falhe.
 */
void wifi_init_sta(void);

/**
 * @brief Inicializa e inicia o cliente MQTT.
 * Deve ser chamada após o Wi-Fi estar conectado (com IP).
 */
void mqtt_app_start(void);

/**
 * @brief Publica uma mensagem MQTT para o tópico especificado.
 * @param topico O tópico MQTT para publicar a mensagem
 * @param payload O conteúdo da mensagem a ser publicada
 */
void mqtt_publicar_mensagem(const char* topico, const char* payload);

/**
 * @brief Atualiza o intervalo de telemetria com o valor recebido via MQTT.
 * @param segundos O novo intervalo em segundos
 */
void atualizar_intervalo_telemetria(int segundos);