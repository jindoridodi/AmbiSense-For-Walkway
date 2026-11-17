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

#define WALKWAY_CONFIG_VERSION 1
/* Stored as one blob so version and geometry cannot be partially updated. */
typedef struct {
    uint32_t version;
    uint8_t enabled;
    walkway_config_t config;
} walkway_saved_t;

/* NULL means valid; otherwise a static explanation. */
const char *walkway_validate(const walkway_config_t *config);
/* Returns whether a radar-relative target is in the corridor. */
bool walkway_project(const walkway_config_t *config, float x_cm, float y_cm, float *path_cm);

/* Returns false on invalid configuration or insufficient output capacity. */
bool walkway_render(const walkway_config_t *config, const radar_frame_t *frame,
                    uint8_t (*rgb)[3], size_t capacity);
