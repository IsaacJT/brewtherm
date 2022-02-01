#pragma once

#include <stdint.h>

enum oled_update_type {
        OLED_UPDATE_NOP,
        OLED_UPDATE_TEMPERATURE,
};

struct oled_update {
        enum oled_update_type type;
        uint8_t *buffer;
        size_t len;
};

struct oled_update_temperature {
        int16_t integer;
        uint16_t fractional;
};

void oled_init(void);
void oled_update(struct oled_update *update);
