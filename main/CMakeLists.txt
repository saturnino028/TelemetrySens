idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES RedesWireless Sensores esp_driver_gpio esp_driver_tsens)