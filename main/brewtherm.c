#include <stdio.h>

#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "oled.h"
#include "max6675.h"

int init_max6675(void);

void app_main(void)
{
	int max6675_inst = init_max6675();
	oled_init();

	for (;;) {
		vTaskDelay(portMAX_DELAY);
	}
}

int init_max6675(void) {
	struct max6675 max6675 = {
		.spi = HSPI_HOST,
	};

	return max6675_init(&max6675);
}
