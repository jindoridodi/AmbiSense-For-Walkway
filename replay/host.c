#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "radar.h"
#include "walkway.h"
#include "led_strip.h"
#include "freertos/semphr.h"

uint8_t replay_rgb[1500][3];
walkway_saved_t replay_walkway;
radar_frame_t replay_radar;
uint64_t replay_now;
int64_t esp_timer_get_time(void) { return replay_now; }
int xTaskCreate(void (*fn)(void *), const char *name, unsigned stack, void *arg, unsigned priority, void *handle) { (void)fn;(void)name;(void)stack;(void)arg;(void)priority;(void)handle; return 0; }
void vTaskDelay(unsigned ticks) { (void)ticks; }
SemaphoreHandle_t xSemaphoreCreateMutex(void) { return NULL; }
int xSemaphoreTake(SemaphoreHandle_t lock, unsigned timeout) { (void)lock;(void)timeout; return 0; }
int xSemaphoreGive(SemaphoreHandle_t lock) { (void)lock; return 0; }
static const char *setting(const char *ns, const char *key) {
    char name[96]; snprintf(name, sizeof name, "AMBI_%s_%s", ns, key);
    return getenv(name);
}
int settings_get_u8(const char *ns, const char *key, uint8_t *out) { const char *s=setting(ns,key); if (!s) return 1; *out=(uint8_t)strtoul(s,NULL,10); return 0; }
int settings_get_u32(const char *ns, const char *key, uint32_t *out) { const char *s=setting(ns,key); if (!s) return 1; *out=(uint32_t)strtoul(s,NULL,10); return 0; }
int settings_get_i32(const char *ns, const char *key, int32_t *out) { const char *s=setting(ns,key); if (!s) return 1; *out=(int32_t)strtol(s,NULL,10); return 0; }
int settings_get_str(const char *ns, const char *key, char *out, size_t max) { const char *s=setting(ns,key); if (!s) return 1; snprintf(out,max,"%s",s); return 0; }
int settings_get_blob(const char *ns, const char *key, void *out, size_t *len) {
    (void)ns; (void)key; if (!replay_walkway.version || *len < sizeof replay_walkway) return 1;
    memcpy(out, &replay_walkway, sizeof replay_walkway); *len = sizeof replay_walkway; return 0;
}
int settings_get_radar_kind(char *out, size_t max) { (void)out;(void)max;return 1; }
int radar_peek(radar_frame_t *f) { *f = replay_radar; return 0; }
int radar_read(radar_frame_t *f, TickType_t timeout) { (void)f;(void)timeout;return 1; }
int led_strip_clear(led_strip_handle_t strip) { (void)strip; memset(replay_rgb, 0, sizeof replay_rgb);return 0; }
int led_strip_set_pixel(led_strip_handle_t strip, int i, uint8_t r, uint8_t g, uint8_t b) { (void)strip;replay_rgb[i][0]=r;replay_rgb[i][1]=g;replay_rgb[i][2]=b;return 0; }
int led_strip_refresh(led_strip_handle_t strip) { (void)strip;return 0; }
int led_strip_del(led_strip_handle_t strip) { (void)strip;return 0; }
int led_strip_new_rmt_device(const led_strip_config_t *c, const led_strip_rmt_config_t *r, led_strip_handle_t *s) { (void)c;(void)r;(void)s;return 0; }
