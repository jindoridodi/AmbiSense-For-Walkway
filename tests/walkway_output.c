#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "radar.h"
#include "walkway.h"
extern walkway_saved_t replay_walkway;
extern radar_frame_t replay_radar;
extern uint64_t replay_now;
extern uint8_t replay_rgb[1500][3];
void replay_led_reset(void);
void replay_led_render(void);

int main(void) {
    replay_walkway = (walkway_saved_t){.version = 1, .enabled = 1, .config = {
        .length_cm = 100, .width_cm = 60, .footprint_cm = 1,
        .side = {{100, 6, 0, false}, {150, 11, 6, false}},
        .chain_count = 17, .brightness = 80, .red = 255, .green = 128, .blue = 0,
    }};
    radar_ld2450_state_t parser = {0};
    uint8_t input[30] = {0xaa,0xff,3,0, 0,0, 0,0x80, 0,0, 1,0};
    input[28]=0x55; input[29]=0xcc;
    const int positions[] = {0, 40, 100, 40, 0};
    for (int directions = 0; directions < 4; ++directions) {
        replay_walkway.config.side[0].reversed = directions & 1;
        replay_walkway.config.side[1].reversed = directions & 2;
        replay_led_reset();
        for (int p = 0; p < 5; ++p) {
            int mm = positions[p] * 10;
            input[6] = mm & 255; input[7] = 0x80 | (mm >> 8);
            replay_now += 1000;
            assert(radar_ld2450_parse_at(&parser, input, sizeof input, &replay_radar, replay_now) == 30);
            replay_led_render();
            int a = positions[p] / 20, b = positions[p] / 10;
            if (directions & 1) a = 5 - a;
            if (directions & 2) b = 10 - b;
            b += 6;
            for (int i = 0; i < 17; ++i) {
                assert(replay_rgb[i][0] == ((i == a || i == b) ? 80 : 0));
                assert(replay_rgb[i][1] == ((i == a || i == b) ? 40 : 0));
                assert(replay_rgb[i][2] == 0);
            }
        }
    }
    replay_walkway.config.side[0].count = 1;
    replay_walkway.config.side[0].reversed = false;
    replay_led_reset();
    for (int y = 0; y <= 100; y += 100) {
        int mm = y * 10;
        input[6] = mm & 255; input[7] = 0x80 | (mm >> 8);
        assert(radar_ld2450_parse_at(&parser, input, sizeof input, &replay_radar, replay_now) == 30);
        replay_led_render();
        assert(replay_rgb[0][0] == 80);
    }
    replay_walkway.config.side[0].count = 6;
    replay_radar.targets[0].x_cm = 31; replay_led_render();
    for (int i = 0; i < 17; ++i) assert(!replay_rgb[i][0]);
    replay_walkway.config.side[1].offset = 5;
    assert(walkway_validate(&replay_walkway.config));
    replay_walkway.config.side[1].offset = 6;
    replay_walkway.config.side[1].count = 12;
    assert(walkway_validate(&replay_walkway.config));
    replay_walkway.config.side[1].count = 11;
    replay_now += 250001; replay_led_render();
    for (int i = 0; i < 17; ++i) assert(!replay_rgb[i][0]);
}
