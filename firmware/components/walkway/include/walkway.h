#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "radar.h"

typedef struct {
    float length_cm;
    uint16_t count, offset;
    bool reversed;
} walkway_side_t;

typedef struct {
    float length_cm, width_cm, tolerance_cm, footprint_cm;
    float radar_x_cm, radar_y_cm, radar_angle_deg;
    walkway_side_t side[2];
    uint16_t chain_count;
    uint8_t brightness, red, green, blue;
} walkway_config_t;

/* Returns false on invalid configuration or insufficient output capacity. */
bool walkway_render(const walkway_config_t *config, const radar_frame_t *frame,
                    uint8_t (*rgb)[3], size_t capacity);
