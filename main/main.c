#include "main.h"

void app_main(void)
{   
    esp_reset_reason_t motivo_reset = esp_reset_reason();
    printf("Iniciando Heltec V3 Telemetry (Master)...\n");

    // Verifica se o boot atual ocorreu por Watchdog
    if (motivo_reset == ESP_RST_TASK_WDT || motivo_reset == ESP_RST_PANIC) {
        total_restarts_watchdog++;
    }

    mutex_botao = xSemaphoreCreateMutex();

    wifi_init_sta();
    sincronizar_tempo_sntp();
    mqtt_app_start();
    
    if (motivo_reset == ESP_RST_TASK_WDT || motivo_reset == ESP_RST_PANIC) {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
        char json_alerta[128];
        snprintf(json_alerta, sizeof(json_alerta),
                 "{\"alerta\": \"Reset by Watchdog\", \"data_hora\": \"%s\"}",
                 timestamp);
        vTaskDelay(pdMS_TO_TICKS(1000));
        mqtt_publicar_mensagem("metadados", json_alerta);
    }
    
    ESP_ERROR_CHECK(modbus_master_init());

    // Inicializa o sensor de temperatura interno do ESP32-S3
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(20, 100);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_sensor));
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));

    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 30000, //Define watchdog timeout para 30 segundos
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };
    ESP_ERROR_CHECK(esp_task_wdt_reconfigure(&twdt_config));
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    xTaskCreate(task_botao, "task_botao", 4096, NULL, 5, NULL);

    float temp = 0.0f;
    float hum = 0.0f;
    float chip_temp = 0.0f;
    int8_t wifi_rssi = 0;
    char json_payload[256];
    bool estado_copia = 0;
    
    TickType_t ultimo_envio = xTaskGetTickCount();

    while (1) {
        esp_task_wdt_reset();

        TickType_t agora = xTaskGetTickCount();
        
        if ((agora - ultimo_envio) >= pdMS_TO_TICKS(intervalo_telemetria_ms)) {
            ultimo_envio = agora;

            // Tenta ler o sensor via Modbus e contabiliza falhas caso ocorra erro
            if (modbus_ler_sensor(&temp, &hum) == ESP_OK) {
                if (xSemaphoreTake(mutex_botao, portMAX_DELAY) == pdTRUE) {
                    estado_copia = estado_atual_botao;
                    xSemaphoreGive(mutex_botao);
                }

                // Lê a temperatura interna do chip ESP32-S3
                temperature_sensor_get_celsius(temp_sensor, &chip_temp);

                // Obtém as informações de conexão do Wi-Fi para ler o RSSI
                wifi_ap_record_t ap_info;
                if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                    wifi_rssi = ap_info.rssi;
                } else {
                    wifi_rssi = 0;
                }

                time_t now;
                struct tm timeinfo;
                time(&now);
                localtime_r(&now, &timeinfo);
                char timestamp[32];
                strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

                // Monta o JSON principal
                snprintf(json_payload, sizeof(json_payload), 
                         "{\"temperatura_ar\": %.1f, \"umidade_ar\": %.1f, \"botao\": %d, "
                         "\"temp_chip\": %.1f, \"erros_modbus\": %lu, \"watchdog_resets\": %lu, "
                         "\"wifi_rssi\": %d, \"data_hora\": \"%s\"}", 
                         temp, hum, estado_copia, chip_temp, contador_erros_modbus, 
                         total_restarts_watchdog, wifi_rssi, timestamp);
                
                mqtt_publicar_mensagem("telemetry_data", json_payload);
            } else {
                // Incrementa a contagem de falhas quando a leitura do barramento falha
                contador_erros_modbus++;
                ESP_LOGW("MAIN", "Falha de leitura Modbus. Total de erros acumulados: %lu", contador_erros_modbus);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void task_botao(void *pvParameters) {
    gpio_reset_pin(BOTAO_PIN);
    gpio_set_direction(BOTAO_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOTAO_PIN, GPIO_PULLDOWN_ONLY);

    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    bool estado_anterior = 0;

    while (1) {
        esp_task_wdt_reset();

        bool leitura_pino = gpio_get_level(BOTAO_PIN);

        if (xSemaphoreTake(mutex_botao, portMAX_DELAY) == pdTRUE) {
            estado_atual_botao = leitura_pino;
            xSemaphoreGive(mutex_botao);
        }

        if (estado_anterior == 0 && leitura_pino == 1) {
            mqtt_publicar_mensagem("telemetry_data", "{\"evento\": \"botao_pressionado\"}");
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        estado_anterior = leitura_pino;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void sincronizar_tempo_sntp(void) {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "a.st1.ntp.br");
    esp_sntp_setservername(1, "pool.ntp.org");
    esp_sntp_init();

    setenv("TZ", "BRT3", 1);
    tzset();

    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < 10) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}