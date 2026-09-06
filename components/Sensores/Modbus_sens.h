#pragma once
/** @file Modbus_sens.h
 *  @brief Declaração das funções para comunicação Modbus RTU com sensores.
 */

#include "esp_err.h"
#include "driver/uart.h"

#define MB_UART_PORT      UART_NUM_1
#define MB_UART_TX_PIN    41
#define MB_UART_RX_PIN    42
#define MB_UART_RTS_PIN   45
#define MB_SLAVE_ADDR     1

/*
@brief Inicializa o mestre Modbus RTU.
@return ESP_OK se a inicialização for bem-sucedida, caso contrário retorna um código de erro.
*/
esp_err_t modbus_master_init(void);

/**
 * @brief Lê os valores dos sensores de temperatura e umidade via Modbus RTU.
 * @param temperatura Ponteiro para armazenar o valor da temperatura.
 * @param umidade Ponteiro para armazenar o valor da umidade.
 * @return ESP_OK se a leitura for bem-sucedida, caso contrário retorna um código de erro.
 * @details Esta função envia uma solicitação Modbus para o dispositivo escravo e aguarda a resposta. 
 * o sensor implementado foi o AHT20.
 * */
esp_err_t modbus_ler_sensor(float *temperatura, float *umidade);