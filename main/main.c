#include "main.h"
#include "MQTT_Com.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    printf("Iniciando Heltec V3 Telemetry...\n");

    // Inicializa o Wi-Fi e aguarda o IP
    wifi_init_sta();

    // Inicia o cliente MQTT
    mqtt_app_start();

    while (1) {
        mqtt_publicar_mensagem("heltec/telemetria", "{\"status\":\"online\", \"dados\": 42}");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}