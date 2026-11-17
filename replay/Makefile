CC ?= cc
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -DAMBISENSE_HOST_REPLAY -Iinclude -I../firmware/components/radar/include -I../firmware/components/motion/include -I../firmware/components/settings/include -I../firmware/components/led_engine/include
CFLAGS = -std=c11 -Wall -Wextra
LDLIBS = -lm
SOURCES = host.c ../firmware/components/radar/radar_ld2450.c ../firmware/components/motion/motion.c ../firmware/components/motion/motion_kalman.c ../firmware/components/led_engine/led_engine.c
.PHONY: all test clean FORCE
all: ambisense-replay
build_identity.h: FORCE
	@printf '#define BUILD_COMMIT "%s"\n#define BUILD_DIRTY "%s"\n' "$$(git rev-parse HEAD)" "$$(if test -n "$$(git status --porcelain)"; then echo yes; else echo no; fi)" > $@
ambisense-replay: main.c $(SOURCES) build_identity.h
	$(CC) $(CPPFLAGS) $(CFLAGS) main.c $(SOURCES) $(LDLIBS) -o $@
test: ../tests/replay_integration.c $(SOURCES)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDLIBS) -o /tmp/ambisense-replay-test
	/tmp/ambisense-replay-test
	$(CC) -std=c11 -Wall -Wextra -Werror -I../tests/stubs -I../firmware/components/radar/include ../tests/ld2450_replay.c ../firmware/components/radar/radar_ld2450.c -o /tmp/ambisense-ld2450-test
	/tmp/ambisense-ld2450-test
clean:
	rm -f ambisense-replay build_identity.h
