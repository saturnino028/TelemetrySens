#pragma once
/** @file main.h
 *  @brief Header file for main.c
 *  @description This file contains the declarations and definitions for the main.c file.
 **/
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "driver/gpio.h"
#include "driver/temperature_sensor.h"
#include "MQTT_Com.h"
#include "Modbus_sens.h"
#include "esp_task_wdt.h"

#define BOTAO_PIN 48 //--GPIO48
bool estado_atual_botao = 0;
SemaphoreHandle_t mutex_botao = NULL;

// Variável global para armazenar o intervalo atual (padrão de 5000 ms)
volatile uint32_t intervalo_telemetria_ms = 5000;

// Contador de falhas consecutivas/totais de leitura no barramento Modbus
uint32_t contador_erros_modbus = 0;

// Contador persistente de boots por Watchdog (armazenado em memória ou contado por boot)
uint32_t total_restarts_watchdog = 0;

// Handle para o sensor de temperatura interno do ESP32-S3
static temperature_sensor_handle_t temp_sensor = NULL; //

/**
 * @brief Task to monitor the button state and publish an MQTT message when pressed.
 * @param pvParameters Pointer to task parameters (not used).
 */
void task_botao(void *pvParameters);

/**
 * @brief Synchronize the system time with SNTP.
 */
void sincronizar_tempo_sntp(void);
