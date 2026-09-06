#include "Modbus_sens.h"
#include "esp_log.h"
#include "mbcontroller.h"

static const char *TAG = "MODBUS_MASTER";
static void *master_handle = NULL;

esp_err_t modbus_master_init(void)
{
    mb_communication_info_t comm_info = {
        .ser_opts.port = MB_UART_PORT,
        .ser_opts.mode = MB_RTU,
        .ser_opts.baudrate = 9600,
        .ser_opts.parity = UART_PARITY_DISABLE,
        .ser_opts.data_bits = UART_DATA_8_BITS,
        .ser_opts.stop_bits = UART_STOP_BITS_1
    };

    ESP_ERROR_CHECK(mbc_master_create_serial(&comm_info, &master_handle));
    ESP_ERROR_CHECK(mbc_master_start(master_handle));

    ESP_ERROR_CHECK(uart_set_pin(MB_UART_PORT, MB_UART_TX_PIN, MB_UART_RX_PIN, MB_UART_RTS_PIN, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(MB_UART_PORT, UART_MODE_RS485_HALF_DUPLEX));

    ESP_LOGI(TAG, "Modbus Master RTU inicializado com sucesso.");
    return ESP_OK;
}

esp_err_t modbus_ler_sensor(float *temperatura, float *umidade)
{
    mb_param_request_t req = {
        .slave_addr = MB_SLAVE_ADDR,
        .command = 3,           // Função 03: Read Holding Registers
        .reg_start = 0,         // Registrador inicial
        .reg_size = 2           // Quantidade de registradores
    };

    int16_t rx_data[2] = {0};   // Buffer para receber os int16_t do Slave

    esp_err_t err = mbc_master_send_request(master_handle, &req, (void*)rx_data);
    
    if (err == ESP_OK) {
        // Desfaz a normalização (divide por 10.0) para recuperar o ponto flutuante
        *temperatura = (float)rx_data[0] / 10.0f;
        *umidade = (float)rx_data[1] / 10.0f;
        ESP_LOGI(TAG, "Leitura OK -> Temp: %.1f, Umi: %.1f", *temperatura, *umidade);
    } else {
        ESP_LOGE(TAG, "Falha na leitura Modbus: %s", esp_err_to_name(err));
    }

    return err;
}