#include "pin_map.h"
#include "eagle_soc.h"
#if 0
uint32_t pin_mux[GPIO_PIN_NUM] = {PERIPHS_IO_MUX_MTDI_U,  PERIPHS_IO_MUX_MTCK_U,  PERIPHS_IO_MUX_MTMS_U, 	 PERIPHS_IO_MUX_MTDO_U,
								  PERIPHS_IO_MUX_U0RXD_U, PERIPHS_IO_MUX_U0TXD_U, PERIPHS_IO_MUX_SD_DATA2_U, PERIPHS_IO_MUX_SD_DATA3_U,
								  PERIPHS_IO_MUX_GPIO0_U, PERIPHS_IO_MUX_GPIO2_U, PERIPHS_IO_MUX_GPIO4_U, 	 PERIPHS_IO_MUX_GPIO5_U};

uint8_t pin_num[GPIO_PIN_NUM] = {12, 13, 14, 15,
								  3,  1,  9, 10,
								  0,  2,  4,  5};

uint8_t pin_func[GPIO_PIN_NUM] = {FUNC_GPIO12, FUNC_GPIO13, FUNC_GPIO14, FUNC_GPIO15,
								  FUNC_GPIO3,  FUNC_GPIO1,  FUNC_GPIO9,  FUNC_GPIO10,
								  FUNC_GPIO0,  FUNC_GPIO2,  FUNC_GPIO4,  FUNC_GPIO5};

#ifdef GPIO_INTERRUPT_ENABLE
GPIO_INT_TYPE pin_int_type[GPIO_PIN_NUM] = {
								GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE,
								GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE,
								GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE};
#endif
#else
uint32_t pin_mux[GPIO_PIN_NUM] = {PAD_XPD_DCDC_CONF,  PERIPHS_IO_MUX_MTMS_U,  PERIPHS_IO_MUX_MTDI_U, 	 PERIPHS_IO_MUX_MTCK_U,
								  PERIPHS_IO_MUX_MTDO_U, PERIPHS_IO_MUX_GPIO2_U, PERIPHS_IO_MUX_GPIO0_U, PERIPHS_IO_MUX_GPIO4_U,
								  PERIPHS_IO_MUX_GPIO5_U, PERIPHS_IO_MUX_U0RXD_U, PERIPHS_IO_MUX_U0TXD_U, PERIPHS_IO_MUX_SD_DATA2_U,
								  PERIPHS_IO_MUX_SD_DATA3_U };

uint8_t pin_num[GPIO_PIN_NUM] = {16, 14, 12, 13,
								 15,  2,  0,  4,
								  5,  3,  1,  9,
								 10};

uint8_t pin_func[GPIO_PIN_NUM] = {0, FUNC_GPIO14, FUNC_GPIO12, FUNC_GPIO13,
								  FUNC_GPIO15,  FUNC_GPIO2,  FUNC_GPIO0,  FUNC_GPIO4,
								  FUNC_GPIO5,  FUNC_GPIO3,  FUNC_GPIO1, FUNC_GPIO9,
								  FUNC_GPIO10};

#ifdef GPIO_INTERRUPT_ENABLE
GPIO_INT_TYPE pin_int_type[GPIO_PIN_NUM] = {
								GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE,
								GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE,
								GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE,
								GPIO_PIN_INTR_DISABLE};
#endif
#endif
