import copy
import ctypes as C
from pathlib import Path
from helper import Config, DEFAULT, make_config, render

lib = C.CDLL(str(Path(__file__).resolve().parent / 'libwalkway.so'))
lib.walkway_validate.argtypes = [C.POINTER(Config)]
lib.walkway_validate.restype = C.c_char_p
lib.helper_render.argtypes = [C.POINTER(Config), C.c_int, C.c_int, C.POINTER(C.c_float), C.POINTER(C.c_float), C.POINTER(C.c_float), C.POINTER(C.c_int), C.POINTER(C.c_uint8)]
lib.helper_render.restype = C.c_int
config = make_config(DEFAULT, lib)
x, y, accepted, path, rgb = render(lib, config, 0, 409)
assert (x, y, accepted, path) == (0, 40, True, 40)
assert [i for i, color in enumerate(rgb) if color != (0, 0, 0)] == [2, 12]
assert rgb[2] == (80, 80, 80)
assert not render(lib, config, 310, 400)[2]
for edit in ('version', 'overlap'):
    data = copy.deepcopy(DEFAULT)
    if edit == 'version': data['version'] = 2
    else: data['side'][1]['offset'] = 5
    try: make_config(data, lib)
    except ValueError: pass
    else: raise AssertionError(f'{edit} accepted')
print('helper parser, preview and validation checks passed')
