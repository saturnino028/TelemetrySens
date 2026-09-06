idf_component_register(SRCS "MQTT_Com.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp-tls esp_wifi esp_event nvs_flash esp_netif mbedtls)