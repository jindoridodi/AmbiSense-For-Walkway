#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "radar.h"
#include "build_identity.h"

extern uint8_t replay_rgb[1500][3];
void replay_motion_reset(void);
void replay_motion_input(const radar_frame_t *frame);
void replay_led_reset(void);
void replay_led_render(void);

int main(void) {
    char line[1024], kind, hex[1000];
    uint64_t now, previous = 0;
    radar_ld2450_state_t parser = {0};
    uint8_t held[512]; size_t held_len = 0;
    fprintf(stderr, "source: %s; dirty at build: %s\n", BUILD_COMMIT, BUILD_DIRTY);
    const char *mode = getenv("AMBI_led_mode");
    if (mode && strcmp(mode, "0") != 0) { fprintf(stderr, "only standard mode is supported\n"); return 1; }
    replay_motion_reset(); replay_led_reset();
    while (fgets(line, sizeof line, stdin)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        hex[0] = 0;
        if (sscanf(line, " %c %" SCNu64 " %999s", &kind, &now, hex) < 2 || now < previous) {
            fprintf(stderr, "invalid or out-of-order event: %s", line); return 1;
        }
        previous = now;
        if (kind == 'I') {
            uint8_t bytes[512]; size_t n = strlen(hex);
            if (!n || n % 2 || n / 2 > sizeof bytes) { fprintf(stderr, "invalid input bytes\n"); return 1; }
            for (size_t i = 0; i < n / 2; i++) {
                unsigned value; char part[3] = {hex[2*i], hex[2*i+1], 0};
                char *end; value = (unsigned)strtoul(part, &end, 16);
                if (*end || end != part + 2) { fprintf(stderr, "invalid input bytes\n"); return 1; }
                bytes[i] = (uint8_t)value;
            }
            if (held_len + n / 2 > sizeof held) { fprintf(stderr, "input buffer overflow\n"); return 1; }
            memcpy(held + held_len, bytes, n / 2); held_len += n / 2;
            radar_frame_t frame = {0};
            size_t used = radar_ld2450_parse_at(&parser, held, held_len, &frame, now);
            if (used) {
                replay_motion_input(&frame);
                memmove(held, held + used, held_len - used); held_len -= used;
            }
        } else if (kind == 'R' && !hex[0]) {
            replay_led_render();
            const char *count = getenv("AMBI_led_count");
            unsigned long n = count ? strtoul(count, NULL, 10) : 30;
            if (n < 1) n = 30;
            if (n > 1500) n = 1500;
            printf("%" PRIu64, now);
            for (unsigned long i=0; i<n; ++i) printf(" %02x%02x%02x", replay_rgb[i][0], replay_rgb[i][1], replay_rgb[i][2]);
            putchar('\n');
        } else { fprintf(stderr, "invalid event: %s", line); return 1; }
    }
    return ferror(stdin) ? 1 : 0;
}
