# Replay baseline

Issue #2 starts from upstream `v6.2.0-alpha.5` (`6f988b3`), merged in
`6f0f542`. Both commits have tree `026d2f782aec4aab90a24a277e6ea123d5c2f648`.
This document describes that baseline, not a completed replay implementation.

- The LD2450 parser decodes signed millimetres and truncates coordinates to
  centimetres. It selects the first nonempty target's radial distance for
  motion processing, keeps previous distance in static state, and reads the
  ESP clock directly. The parser now also exposes the same decoding with
  caller-provided state and time for replay.
- Motion processing consumes radar frames in a FreeRTOS task, clamps radial
  distance and maintains filter state. The renderer independently reads the
  latest motion state on a roughly 16 ms schedule.
- The standard renderer maps distance to a pixel span and scales each RGB
  channel by global brightness before writing to the strip. It does not use
  the target's presence flag to extinguish the span. This is baseline
  behavior, not a walkway target-loss implementation.
- Fire and motion particles use `rand()`. Their output is not reproducible
  from timestamps and configuration alone without controlling random state.
- The baseline simulator supplies scalar distances, not LD2450 protocol
  frames. It cannot validate coordinate decoding or quantization.
- The LD2450 integer square root starts at `1 << 14`. For squared distances
  above its supported range it can underreport the radial distance (for
  example, 500 cm on one axis produces 255 cm). This is a baseline defect.

The host/ESP shared build, explicit replay state and clock, final RGB capture,
protocol fixtures, source identity reporting, and device comparison are still
required. No hardware validation is implied by this inventory.

The parser fixture can be run on a host with a C compiler:

```sh
cc -std=c11 -Wall -Wextra -Werror -Itests/stubs \
  -Ifirmware/components/radar/include tests/ld2450_replay.c \
  firmware/components/radar/radar_ld2450.c -o /tmp/ambisense-ld2450-test
/tmp/ambisense-ld2450-test
```
