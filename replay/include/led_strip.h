#pragma once
#include <stdint.h>
typedef void *led_strip_handle_t;
typedef struct { int strip_gpio_num, max_leds, led_model, color_component_format; struct { int invert_out; } flags; } led_strip_config_t;
typedef struct { int clk_src, resolution_hz; struct { int with_dma; } flags; } led_strip_rmt_config_t;
#define LED_MODEL_WS2812 0
#define LED_STRIP_COLOR_COMPONENT_FMT_GRB 0
#define RMT_CLK_SRC_DEFAULT 0
int led_strip_clear(led_strip_handle_t strip);
int led_strip_set_pixel(led_strip_handle_t strip, int i, uint8_t r, uint8_t g, uint8_t b);
int led_strip_refresh(led_strip_handle_t strip);
int led_strip_del(led_strip_handle_t strip);
int led_strip_new_rmt_device(const led_strip_config_t *config, const led_strip_rmt_config_t *rmt, led_strip_handle_t *strip);
