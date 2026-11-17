# Native baseline replay (experimental)

`make -C replay` builds the actual LD2450 parser, motion processing and LED
engine source with host substitutes for ESP services. `make -C replay test`
runs the protocol and input-to-RGB checks. A C compiler and make are required.
The program prints the Git HEAD and whether the working tree was dirty **when
built** to stderr; rebuild after any change. A dirty build cannot be identified
by its commit alone. Firmware builds report `source_commit` and `source_dirty`
through `/api/version` and `/api/v1/info`; `unknown` means Git was unavailable
at build time. A dirty build still needs its changes preserved to be reproducible.

Events on stdin have nondecreasing microsecond timestamps:

- `I <time> <hex>` supplies one UART read as hex bytes. The baseline radar
  loop calls the parser once per read and retains the unconsumed bytes.
- `P <time>` runs another radar loop iteration with no new UART bytes. Use it
  when a read contains more than one frame; the firmware parses once per loop.
- `R <time>` renders once. Output is the timestamp followed by one `RRGGBB`
  value per LED, in physical chain index order, after brightness scaling.

State starts at zero, with firmware default settings. Override NVS values with
`AMBI_<namespace>_<key>` environment variables. Example:

```sh
printf 'I 1000 aaff03000000e883000001000000000000000000000000000000000055cc\nR 1000\n' |
  env AMBI_led_count=10 AMBI_led_span=2 AMBI_motion_en=0 AMBI_motion_outl=0 \
      AMBI_dist_min=0 AMBI_dist_max=200 replay/ambisense-replay
```

This models one deterministic ordering of radar and render events, not the
asynchronous device task scheduler. Standard, rainbow and solid modes are
supported; other effects are not validated for replay. The host substitutes assume settings are
valid values of the same types as NVS. The firmware's actual settings, timing,
LED output must still be captured independently for a
comparison. Physical hardware validation is still required.

Since this change, incomplete LD2450 packets are retained across reads, and discarded malformed bytes do not publish a frame. Replay matches the corrected firmware behavior; earlier baseline captures may differ. Standard mode
also continues to render when the target is absent. These are baseline limits,
not walkway loss handling. The host currently rejects an input buffer larger
than 512 bytes instead of reproducing the firmware's overflow recovery.

The production firmware, including the shared parser, motion and renderer,
was built for ESP32-C3 with ESP-IDF v5.3 at `c9ccabb4f9355a4b1e0ad41501e871350cc08c5b`
with a dirty worktree (the added host fixture). To repeat from `firmware/`:

```sh
. "$IDF_PATH/export.sh"
idf.py set-target esp32c3
idf.py build
```

The build confirms compilation, not equivalent device scheduling or physical output.
