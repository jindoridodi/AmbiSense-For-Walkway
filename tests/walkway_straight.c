#include <assert.h>
#include <stdint.h>
#include "radar.h"
#include "walkway.h"
int64_t esp_timer_get_time(void) { return 0; }

int main(void) {
    walkway_config_t config = {
        .length_cm = 100, .width_cm = 60, .footprint_cm = 1,
        .side = {{100, 6, 0, false}, {150, 11, 6, true}},
        .chain_count = 17, .brightness = 80, .red = 255, .green = 255, .blue = 255,
    };
    uint8_t input[30] = {0xaa,0xff,3,0, 0,0, 0x90,0x81, 0,0, 1,0};
    input[28]=0x55; input[29]=0xcc;
    radar_ld2450_state_t parser = {0}; radar_frame_t frame = {0};
    uint8_t rgb[17][3];
    assert(radar_ld2450_parse_at(&parser, input, sizeof input, &frame, 1000) == 30);
    assert(walkway_render(&config, &frame, rgb, 17));
    for (int i = 0; i < 17; ++i)
        for (int c = 0; c < 3; ++c)
            assert(rgb[i][c] == (i == 2 || i == 12 ? 80 : 0));
    input[6] = 0; input[7] = 0x80;
    frame = (radar_frame_t){0};
    assert(radar_ld2450_parse_at(&parser, input, sizeof input, &frame, 2000) == 30);
    assert(walkway_render(&config, &frame, rgb, 17));
    assert(rgb[0][0] == 80 && rgb[16][0] == 80);
    input[6] = 0xe8; input[7] = 0x83;
    frame = (radar_frame_t){0};
    assert(radar_ld2450_parse_at(&parser, input, sizeof input, &frame, 3000) == 30);
    assert(walkway_render(&config, &frame, rgb, 17));
    assert(rgb[5][0] == 80 && rgb[6][0] == 80);
    config.side[1].offset = 5;
    assert(!walkway_render(&config, &frame, rgb, 17));
}
