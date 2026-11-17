#include "walkway.h"
#include <math.h>
#include <string.h>

bool walkway_render(const walkway_config_t *c, const radar_frame_t *f,
                    uint8_t (*rgb)[3], size_t capacity) {
    if (!c || !f || !rgb || !isfinite(c->length_cm) || c->length_cm <= 0 ||
        !isfinite(c->width_cm) || c->width_cm <= 0 ||
        !isfinite(c->tolerance_cm) || c->tolerance_cm < 0 ||
        !isfinite(c->footprint_cm) || c->footprint_cm <= 0 ||
        !isfinite(c->radar_x_cm) || !isfinite(c->radar_y_cm) ||
        !isfinite(c->radar_angle_deg) || c->chain_count == 0 ||
        c->chain_count > 1500 || capacity < c->chain_count) return false;
    for (int s = 0; s < 2; ++s) {
        const walkway_side_t *side = &c->side[s];
        if (!isfinite(side->length_cm) || side->length_cm <= 0 || side->count == 0 ||
            (unsigned)side->offset + side->count > c->chain_count) return false;
    }
    if (c->side[0].offset < (unsigned)c->side[1].offset + c->side[1].count &&
        c->side[1].offset < (unsigned)c->side[0].offset + c->side[0].count) return false;
    memset(rgb, 0, c->chain_count * sizeof *rgb);
    float angle = c->radar_angle_deg * (3.14159265358979323846f / 180.f);
    for (int t = 0; t < RADAR_MAX_TARGETS; ++t) {
        if (!f->targets[t].resolution_mm && !f->targets[t].x_cm &&
            !f->targets[t].y_cm && !f->targets[t].v_cms) continue;
        float x = c->radar_x_cm + f->targets[t].x_cm * cosf(angle) - f->targets[t].y_cm * sinf(angle);
        float y = c->radar_y_cm + f->targets[t].x_cm * sinf(angle) + f->targets[t].y_cm * cosf(angle);
        if (fabsf(x) > c->width_cm / 2 + c->tolerance_cm || y < 0 || y > c->length_cm) continue;
        for (int s = 0; s < 2; ++s) {
            const walkway_side_t *side = &c->side[s];
            for (unsigned i = 0; i < side->count; ++i) {
                float p = side->count == 1 ? 0 : (float)i / (side->count - 1);
                float half_cell = side->count == 1 ? c->length_cm / 2 : c->length_cm / (2 * (side->count - 1));
                if (fabsf(p * c->length_cm - y) > c->footprint_cm / 2 + half_cell) continue;
                unsigned index = side->offset + (side->reversed ? side->count - 1 - i : i);
                rgb[index][0] = (uint8_t)(c->red * (c->brightness / 255.f));
                rgb[index][1] = (uint8_t)(c->green * (c->brightness / 255.f));
                rgb[index][2] = (uint8_t)(c->blue * (c->brightness / 255.f));
            }
        }
        break;
    }
    return true;
}
