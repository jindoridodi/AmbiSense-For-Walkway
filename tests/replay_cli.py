import os
import subprocess

config = os.environ | {
    "AMBI_led_count": "10", "AMBI_led_span": "2", "AMBI_motion_en": "0",
    "AMBI_motion_outl": "0", "AMBI_dist_min": "0", "AMBI_dist_max": "200",
}
target = "aaff03000000e883000001000000000000000000000000000000000055cc"
absent = "aaff0300" + "00" * 24 + "55cc"
malformed = target[:-2] + "00"
trace = f"I 1000 {target}{absent}\nR 1000\nP 2000\nR 2000\nI 3000 {malformed}\nR 3000\n"
result = subprocess.run(["./ambisense-replay"], input=trace, text=True,
                        env=config, capture_output=True, check=True)
assert result.stdout.splitlines() == [
    "1000 000000 000000 000000 000000 505050 505050 000000 000000 000000 000000",
    "2000 505050 505050 000000 000000 000000 000000 000000 000000 000000 000000",
    "3000 505050 505050 000000 000000 000000 000000 000000 000000 000000 000000",
]
repeat = subprocess.run(["./ambisense-replay"], input=trace, text=True,
                        env=config, capture_output=True, check=True)
assert result.stdout == repeat.stdout
incomplete = subprocess.run(
    ["./ambisense-replay"], input=f"I 1000 {target[:-2]}\nR 1000\n",
    text=True, env=config, capture_output=True, check=True,
)
assert incomplete.stdout == (
    "1000 505050 505050 000000 000000 000000 000000 000000 000000 000000 000000\n"
)
solid = subprocess.run(
    ["./ambisense-replay"], input="R 1000\n", text=True,
    env=config | {"AMBI_led_mode": "4"}, capture_output=True, check=True,
)
assert solid.stdout == "1000" + " 505050" * 10 + "\n"
rainbow = subprocess.run(
    ["./ambisense-replay"], input="R 1000\n", text=True,
    env=config | {"AMBI_led_mode": "1"}, capture_output=True, check=True,
)
assert rainbow.stdout.split()[1] == "005000"

# -123 mm, +207 mm become -12 cm, +20 cm, then 23 cm radial distance.
quantized = subprocess.run(
    ["./ambisense-replay"],
    input="I 1000 aaff03007b00cf80058028000000000000000000000000000000000055cc\nR 1000\n",
    text=True, env=config | {"AMBI_dist_max": "48"}, capture_output=True, check=True,
)
assert quantized.stdout == (
    "1000 000000 000000 000000 505050 505050 000000 000000 000000 000000 000000\n"
)

# A fragment and malformed packet must not replace the last valid target.
fragmented = subprocess.run(
    ["./ambisense-replay"],
    input=f"I 1000 {target}\nR 1000\nI 2000 {malformed}\nR 2000\nI 3000 {target[:20]}\nR 3000\nI 4000 {target[20:]}\nR 4000\n",
    text=True, env=config, capture_output=True, check=True,
)
lines = fragmented.stdout.splitlines()
assert len(lines) == 4 and all(line.split()[1:] == lines[0].split()[1:] for line in lines)
