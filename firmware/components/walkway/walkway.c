#include "walkway.h"
#include <math.h>
#include <string.h>

const char *walkway_validate(const walkway_config_t *c) {
    if (!c) return "missing configuration";
    if (!isfinite(c->length_cm) || c->length_cm <= 0 ||
        !isfinite(c->width_cm) || c->width_cm <= 0 ||
        !isfinite(c->tolerance_cm) || c->tolerance_cm < 0 ||
        !isfinite(c->footprint_cm) || c->footprint_cm <= 0 ||
        c->length_cm > 1000000 || c->width_cm > 1000000 ||
        c->tolerance_cm > 1000000 || c->footprint_cm > 1000000)
        return "invalid segment dimensions";
    if (!isfinite(c->radar_x_cm) || !isfinite(c->radar_y_cm) || !isfinite(c->radar_angle_deg) || fabsf(c->radar_x_cm) > 1000000 ||
        fabsf(c->radar_y_cm) > 1000000 || fabsf(c->radar_angle_deg) > 360000)
        return "invalid radar transform";
    if (!c->chain_count || c->chain_count > 1500) return "chain count must be 1..1500";
    for (int s = 0; s < 2; ++s) {
        const walkway_side_t *side = &c->side[s];
        if (!isfinite(side->length_cm) || side->length_cm <= 0 || side->length_cm > 1000000 || !side->count)
            return "invalid side length or count";
        if ((unsigned)side->offset + side->count > c->chain_count) return "side range exceeds chain";
    }
    if (c->side[0].offset < (unsigned)c->side[1].offset + c->side[1].count &&
        c->side[1].offset < (unsigned)c->side[0].offset + c->side[0].count)
        return "side ranges overlap";
    return NULL;
}

bool walkway_project(const walkway_config_t *c, float x_cm, float y_cm, float *path_cm) {
    if (walkway_validate(c) || !isfinite(x_cm) || !isfinite(y_cm)) return false;
    float angle = c->radar_angle_deg * (3.14159265358979323846f / 180.f);
    float x = c->radar_x_cm + x_cm * cosf(angle) - y_cm * sinf(angle);
    float y = c->radar_y_cm + x_cm * sinf(angle) + y_cm * cosf(angle);
    if (!isfinite(x) || !isfinite(y) || fabsf(x) > c->width_cm / 2 + c->tolerance_cm || y < 0 || y > c->length_cm) return false;
    if (path_cm) *path_cm = y;
    return true;
}

bool walkway_render(const walkway_config_t *c, const radar_frame_t *f,
                    uint8_t (*rgb)[3], size_t capacity) {
    if (walkway_validate(c) || !f || !rgb || capacity < c->chain_count) return false;
    memset(rgb, 0, c->chain_count * sizeof *rgb);
    for (int t = 0; t < RADAR_MAX_TARGETS; ++t) {
        if (!f->targets[t].resolution_mm && !f->targets[t].x_cm &&
            !f->targets[t].y_cm && !f->targets[t].v_cms) continue;
        float y;
        if (!walkway_project(c, f->targets[t].x_cm, f->targets[t].y_cm, &y)) continue;
        for (int s = 0; s < 2; ++s) {
            const walkway_side_t *side = &c->side[s];
            for (unsigned i = 0; i < side->count; ++i) {
                float p = side->count == 1 ? 0.5f : (float)i / (side->count - 1);
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
