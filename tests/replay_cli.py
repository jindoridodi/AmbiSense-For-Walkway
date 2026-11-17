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
