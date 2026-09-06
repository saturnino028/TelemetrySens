idf_component_register(SRCS "Modbus_sens.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp-modbus driver esp_driver_uart)