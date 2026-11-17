CC ?= cc
CFLAGS = -std=c11 -Wall -Wextra -Werror -fPIC
INCLUDES = -I../tests/stubs -I../firmware/components/radar/include -I../firmware/components/walkway/include
SOURCES = bridge.c ../firmware/components/radar/radar_ld2450.c ../firmware/components/walkway/walkway.c

.PHONY: all test clean
all: libwalkway.so
libwalkway.so: $(SOURCES) ../firmware/components/walkway/include/walkway.h
	$(CC) $(CFLAGS) $(INCLUDES) -shared $(SOURCES) -lm -o $@
test: libwalkway.so
	python3 test_helper.py
clean:
	rm -f libwalkway.so
