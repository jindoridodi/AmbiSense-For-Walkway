#include <stdint.h>
#include <string.h>
#include "walkway.h"

int64_t esp_timer_get_time(void) { return 0; }

/* Inputs are signed millimeters, encoded as LD2450 sign-magnitude fields. */
int helper_render(const walkway_config_t *config, int x_mm, int y_mm,
                  float *x_cm, float *y_cm, float *path_cm, int *accepted,
                  uint8_t (*rgb)[3]) {
    if (x_mm < -32767 || x_mm > 32767 || y_mm < -32767 || y_mm > 32767) return 0;
    uint8_t bytes[30] = {0xaa, 0xff, 3, 0};
    int values[] = {x_mm, y_mm};
    for (int i = 0; i < 2; ++i) {
        unsigned value = values[i] < 0 ? (unsigned)-values[i] : (unsigned)values[i] | 0x8000;
        bytes[4 + i * 2] = value & 255;
        bytes[5 + i * 2] = value >> 8;
    }
    bytes[10] = 1; /* Keep a target at the sensor origin present. */
    bytes[28] = 0x55; bytes[29] = 0xcc;
    radar_ld2450_state_t state = {0};
    radar_frame_t frame = {0};
    if (radar_ld2450_parse_at(&state, bytes, sizeof bytes, &frame, 0) != sizeof bytes) return 0;
    *x_cm = frame.targets[0].x_cm;
    *y_cm = frame.targets[0].y_cm;
    *accepted = walkway_project(config, *x_cm, *y_cm, path_cm);
    return walkway_render(config, &frame, rgb, config->chain_count);
}
