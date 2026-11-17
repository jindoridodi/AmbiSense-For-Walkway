#include <assert.h>
#include <string.h>
#include "radar.h"

int64_t esp_timer_get_time(void) { return 123; }

int main(void) {
    const uint8_t input[30] = {
        0xaa, 0xff, 0x03, 0x00,
        0x7b, 0x00, 0xcf, 0x80, 0x05, 0x80, 0x28, 0x00,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0x55, 0xcc
    };
    radar_ld2450_state_t state = {0};
    radar_frame_t frame = {0};
    assert(radar_ld2450_parse_at(&state, input, sizeof input, &frame, 1000) == 30);
    assert(frame.present && frame.target_count == 1);
    assert(frame.targets[0].x_cm == -12 && frame.targets[0].y_cm == 20);
    assert(frame.targets[0].v_cms == 5 && frame.targets[0].resolution_mm == 40);
    assert(frame.distance_cm == 23 && frame.direction == 1 && frame.ts_us == 1000);
    radar_frame_t untouched = frame;
    assert(radar_ld2450_parse_at(&state, input, 29, &frame, 2000) == 26);
    assert(memcmp(&frame, &untouched, sizeof frame) == 0);
    uint8_t malformed[30];
    memcpy(malformed, input, sizeof input);
    malformed[29] = 0;
    assert(radar_ld2450_parse_at(&state, malformed, sizeof malformed, &frame, 2000) == 1);
    assert(memcmp(&frame, &untouched, sizeof frame) == 0);
    uint8_t absent[30] = {0xaa, 0xff, 3, 0};
    absent[28] = 0x55; absent[29] = 0xcc;
    assert(radar_ld2450_parse_at(&state, absent, sizeof absent, &frame, 3000) == 30);
    assert(!frame.present && frame.target_count == 0 && frame.distance_cm == 0);
    assert(frame.direction == -1 && frame.ts_us == 3000);
    uint8_t distant[30] = {0xaa, 0xff, 3, 0, 0, 0, 0x88, 0x93, 0, 0, 1, 0};
    distant[28] = 0x55; distant[29] = 0xcc;
    assert(radar_ld2450_parse_at(&state, distant, sizeof distant, &frame, 4000) == 30);
    assert(frame.targets[0].y_cm == 500 && frame.distance_cm == 255);
}
