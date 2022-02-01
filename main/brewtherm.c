#include <stdio.h>

#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "oled.h"

void app_main(void)
{
	oled_init();

	for (;;) {
		vTaskDelay(portMAX_DELAY);
	}
}
