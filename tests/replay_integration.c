#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "radar.h"
extern uint8_t replay_rgb[1500][3];
void replay_motion_reset(void);
void replay_motion_input(const radar_frame_t *frame);
void replay_led_reset(void);
void replay_led_render(void);
int main(void) {
    uint8_t input[30] = {0xaa,0xff,3,0, 0,0, 0xe8,0x83, 0,0, 1,0};
    input[28]=0x55; input[29]=0xcc;
    setenv("AMBI_led_count", "10", 1);
    setenv("AMBI_led_span", "2", 1);
    setenv("AMBI_motion_en", "0", 1);
    setenv("AMBI_motion_outl", "0", 1);
    setenv("AMBI_dist_min", "0", 1);
    setenv("AMBI_dist_max", "200", 1);
    radar_ld2450_state_t parser = {0}; radar_frame_t frame = {0};
    replay_motion_reset(); replay_led_reset();
    assert(radar_ld2450_parse_at(&parser, input, sizeof input, &frame, 1000) == 30);
    replay_motion_input(&frame); replay_led_render();
    for (int i=0;i<10;i++) for (int c=0;c<3;c++) assert(replay_rgb[i][c] == ((i == 4 || i == 5) ? 80 : 0));
    uint8_t saved[30][3]; memcpy(saved,replay_rgb,sizeof saved);
    parser = (radar_ld2450_state_t){0}; frame = (radar_frame_t){0};
    replay_motion_reset(); replay_led_reset();
    assert(radar_ld2450_parse_at(&parser,input,sizeof input,&frame,1000)==30);
    replay_motion_input(&frame); replay_led_render();
    assert(memcmp(saved,replay_rgb,sizeof saved)==0);
    uint8_t absent[30] = {0xaa,0xff,3,0}; absent[28]=0x55; absent[29]=0xcc;
    assert(radar_ld2450_parse_at(&parser, absent, sizeof absent, &frame, 2000) == 30);
    replay_motion_input(&frame); replay_led_render();
    for (int i=0;i<10;i++) for (int c=0;c<3;c++) assert(replay_rgb[i][c] == (i < 2 ? 80 : 0));

}
