idf_component_register(SRCS "bsp_uart.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash driver mbedtls)
