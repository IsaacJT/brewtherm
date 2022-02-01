#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>

#include "ssd1306.h"

#include "oled.h"

#define PIN_SCL (4)
#define PIN_SDA (5)
#define ADDR (0x3C)
#define QUEUE_LEN (10)
#define STACK_SIZE (1024)

static const char *TAG = "oled";

static void oled_task(void *params);
static void handle_oled_update(struct oled_update *update);
static void handle_oled_update_temperature(struct oled_update *update);
static void update_display(void);
static void set_temperature(int16_t integer, uint16_t fractional);

static struct {
	TaskHandle_t handle;
	QueueHandle_t queue;
	char title[32];
	char temperature[32];
} oled = {
	.handle = NULL,
	.queue = NULL,
	.title = "BrewTherm",
	.temperature = "",
};

void oled_init(void)
{
	if (oled.handle) {
		ESP_LOGW(TAG, "Task already initialised; skipping");
	}
	xTaskCreate(oled_task, "oled", STACK_SIZE, NULL, tskIDLE_PRIORITY+1,
		    &oled.handle);

	oled.queue = xQueueCreate(QUEUE_LEN, sizeof(struct oled_update));
	if (!oled.queue) {
		ESP_LOGE(TAG, "Failed to create queue");
		esp_restart();
	}
}

void oled_update(struct oled_update *update)
{
	if (!update) {
		return;
	}

	if (xQueueSend(oled.queue, update, portMAX_DELAY) != pdPASS) {
		ESP_LOGE(TAG, "Failed to write to queue");
	}
}

static void oled_task(void *params)
{
	ssd1306_128x64_i2c_initEx(PIN_SCL, PIN_SDA, ADDR);
	ssd1306_clearScreen();
	ssd1306_setFixedFont(ssd1306xled_font6x8);

	set_temperature(0, 0);
	update_display();

	for (;;) {
		struct oled_update update;
		if (xQueueReceive(oled.queue, &update, portMAX_DELAY) ==
		    pdPASS) {
			handle_oled_update(&update);
		}
	}
}

static void handle_oled_update(struct oled_update *update)
{
	if (!update) {
		return;
	}

	switch (update->type) {
	case OLED_UPDATE_TEMPERATURE:
		handle_oled_update_temperature(update);
		break;
	case OLED_UPDATE_NOP:
	default:
		break;
	};

	update_display();
}

static void handle_oled_update_temperature(struct oled_update *update)
{
	struct oled_update_temperature *temp;
	if (!update) {
		return;
	}

	if (update->len != sizeof(struct oled_update_temperature)) {
		ESP_LOGW(
			TAG,
			"%s: Unexpected message length, expected %u but got %u",
			__func__, update->len,
			sizeof(struct oled_update_temperature));
		return;
	}

	temp = (struct oled_update_temperature *)update->buffer;
	set_temperature(temp->integer, temp->fractional);
}

static void set_temperature(int16_t integer, uint16_t fractional)
{
	snprintf(oled.temperature, sizeof(oled.temperature), "%3d.%02u C",
		 integer, fractional);
}

static void update_display(void)
{
	ssd1306_printFixed(0, 0, oled.title, STYLE_BOLD);
	ssd1306_printFixedN(0, 32, oled.temperature, STYLE_NORMAL, 1);
}
