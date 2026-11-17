#!/usr/bin/env python3
"""Offline straight walkway preview backed by firmware C code."""
import ctypes as C
import json
import math
from pathlib import Path
import subprocess
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

HERE = Path(__file__).resolve().parent

class Side(C.Structure):
    _fields_ = [('length_cm', C.c_float), ('count', C.c_uint16),
                ('offset', C.c_uint16), ('reversed', C.c_bool)]

class Config(C.Structure):
    _fields_ = [(k, C.c_float) for k in ('length_cm', 'width_cm', 'tolerance_cm',
               'footprint_cm', 'radar_x_cm', 'radar_y_cm', 'radar_angle_deg')] + [
               ('side', Side * 2), ('chain_count', C.c_uint16),
               ('brightness', C.c_uint8), ('red', C.c_uint8), ('green', C.c_uint8), ('blue', C.c_uint8)]

DEFAULT = dict(version=1, enabled=True, length_cm=100, width_cm=60, tolerance_cm=0,
               footprint_cm=1, radar_x_cm=0, radar_y_cm=0, radar_angle_deg=0,
               chain_count=17, brightness=80, red=255, green=255, blue=255,
               side=[dict(length_cm=100, count=6, offset=0, reversed=False),
                     dict(length_cm=150, count=11, offset=6, reversed=True)])
FLOATS = ('length_cm', 'width_cm', 'tolerance_cm', 'footprint_cm', 'radar_x_cm', 'radar_y_cm', 'radar_angle_deg')
INTS = ('chain_count', 'brightness', 'red', 'green', 'blue')
SIDE_FIELDS = ('length_cm', 'count', 'offset', 'reversed')

def load_library():
    subprocess.run(['make', '-s', '-C', str(HERE)], check=True)
    lib = C.CDLL(str(HERE / 'libwalkway.so'))
    lib.walkway_validate.argtypes = [C.POINTER(Config)]
    lib.walkway_validate.restype = C.c_char_p
    lib.helper_render.argtypes = [C.POINTER(Config), C.c_int, C.c_int, C.POINTER(C.c_float),
                                  C.POINTER(C.c_float), C.POINTER(C.c_float), C.POINTER(C.c_int), C.POINTER(C.c_uint8)]
    lib.helper_render.restype = C.c_int
    return lib

def make_config(data, lib):
    if not isinstance(data, dict) or data.get('version') != 1 or type(data.get('enabled')) is not bool:
        raise ValueError('Expected a version 1 configuration with boolean enabled.')
    expected = {'version', 'enabled', 'side', *FLOATS, *INTS}
    if set(data) != expected or not isinstance(data['side'], list) or len(data['side']) != 2:
        raise ValueError('Configuration must contain exactly the version 1 fields and two sides.')
    def number(value, name):
        if type(value) not in (int, float) or not math.isfinite(value):
            raise ValueError(f'{name} must be a finite number.')
        return value
    def integer(value, name, maximum):
        if type(value) is not int or not 0 <= value <= maximum:
            raise ValueError(f'{name} must be an integer between 0 and {maximum}.')
        return value
    c = Config()
    for key in FLOATS:
        setattr(c, key, number(data[key], key))
    for key in INTS:
        setattr(c, key, integer(data[key], key, 65535 if key == 'chain_count' else 255))
    for i, side in enumerate(data['side']):
        if not isinstance(side, dict) or set(side) != set(SIDE_FIELDS) or type(side['reversed']) is not bool:
            raise ValueError(f'Side {i + 1} must have length_cm, count, offset and boolean reversed.')
        c.side[i] = Side(number(side['length_cm'], 'side length_cm'), integer(side['count'], 'count', 65535),
                         integer(side['offset'], 'offset', 65535), side['reversed'])
    error = lib.walkway_validate(C.byref(c))
    if error:
        raise ValueError(error.decode())
    return c

def render(lib, config, x_mm, y_mm):
    if not -32767 <= x_mm <= 32767 or not -32767 <= y_mm <= 32767:
        raise ValueError('Target coordinates must be between -32767 and 32767 mm.')
    rgb = (C.c_uint8 * (config.chain_count * 3))()
    x, y, path = C.c_float(), C.c_float(), C.c_float()
    accepted = C.c_int()
    if not lib.helper_render(C.byref(config), x_mm, y_mm, C.byref(x), C.byref(y), C.byref(path), C.byref(accepted), rgb):
        raise ValueError('Production rendering failed.')
    return x.value, y.value, bool(accepted.value), path.value, [tuple(rgb[i:i+3]) for i in range(0, len(rgb), 3)]

class App:
    def __init__(self, root, lib):
        self.root, self.lib = root, lib
        root.title('AmbiSense straight walkway — offline preview')
        self.vars = {}
        panel = ttk.Frame(root, padding=8); panel.pack(side='left', fill='y')
        ttk.Label(panel, text='Configuration (cm, degrees, zero-based offsets)').pack(anchor='w')
        for key in (*FLOATS, *INTS): self.field(panel, key, DEFAULT[key])
        self.enabled = tk.BooleanVar(value=True)
        ttk.Checkbutton(panel, text='enabled on device', variable=self.enabled).pack(anchor='w')
        for i in range(2):
            ttk.Label(panel, text=f'Side {i + 1}').pack(anchor='w')
            for key in SIDE_FIELDS:
                name = f'side{i}.{key}'
                if key == 'reversed':
                    self.vars[name] = tk.BooleanVar(value=DEFAULT['side'][i][key])
                    ttk.Checkbutton(panel, text='reversed', variable=self.vars[name]).pack(anchor='w')
                else: self.field(panel, name, DEFAULT['side'][i][key])
        self.field(panel, 'target_x_mm', 0); self.field(panel, 'target_y_mm', 400)
        buttons = ttk.Frame(panel); buttons.pack(fill='x', pady=5)
        for text, action in [('Preview', self.preview), ('Open JSON', self.open), ('Save JSON', self.save)]:
            ttk.Button(buttons, text=text, command=action).pack(side='left')
        right = ttk.Frame(root, padding=8); right.pack(side='left', fill='both', expand=True)
        ttk.Label(right, text='Software-only prediction; coverage and physical wiring are unvalidated.\nTurns, tracking, fade and other effects are unsupported.').pack(anchor='w')
        self.status = tk.StringVar(); ttk.Label(right, textvariable=self.status, wraplength=650).pack(anchor='w')
        self.canvas = tk.Canvas(right, width=650, height=280, background='white'); self.canvas.pack(fill='x')
        ttk.Label(right, text='Actual indexed RGB (including unlit LEDs)').pack(anchor='w')
        self.output = tk.Text(right, width=65, height=15, wrap='word'); self.output.pack(fill='both', expand=True)
        try:
            identity = subprocess.check_output(['git', '-C', str(HERE), 'rev-parse', 'HEAD'], text=True).strip()
            dirty = subprocess.check_output(['git', '-C', str(HERE), 'status', '--porcelain'], text=True).strip()
            ttk.Label(right, text=f'Source commit: {identity}'+ (' (working tree modified)' if dirty else '')).pack(anchor='w')
        except (OSError, subprocess.CalledProcessError):
            ttk.Label(right, text='Source commit unavailable; verify firmware compatibility.').pack(anchor='w')
        self.preview()

    def field(self, parent, key, value):
        row = ttk.Frame(parent); row.pack(fill='x')
        ttk.Label(row, text=key, width=20).pack(side='left')
        self.vars[key] = tk.StringVar(value=str(value))
        ttk.Entry(row, textvariable=self.vars[key], width=12).pack(side='left')

    def data(self):
        data = {'version': 1, 'enabled': self.enabled.get(), 'side': []}
        for key in FLOATS: data[key] = float(self.vars[key].get())
        for key in INTS: data[key] = int(self.vars[key].get())
        for i in range(2):
            data['side'].append({'length_cm': float(self.vars[f'side{i}.length_cm'].get()),
                'count': int(self.vars[f'side{i}.count'].get()), 'offset': int(self.vars[f'side{i}.offset'].get()),
                'reversed': self.vars[f'side{i}.reversed'].get()})
        return data

    def preview(self):
        try:
            data = self.data(); config = make_config(data, self.lib)
            x, y, accepted, path, rgb = render(self.lib, config, int(self.vars['target_x_mm'].get()), int(self.vars['target_y_mm'].get()))
            self.status.set(f'Parser target: ({x:g}, {y:g}) cm. ' + (f'Accepted: path position {path:g} cm.' if accepted else 'Rejected: outside corridor or segment.') + (' Preview assumes walkway enabled.' if not data['enabled'] else ''))
            self.output.delete('1.0', 'end')
            self.output.insert('end', '\n'.join(f'{i}: {r}, {g}, {b}' for i, (r, g, b) in enumerate(rgb)))
            self.canvas.delete('all')
            self.canvas.create_rectangle(50, 90, 600, 190, outline='black')
            self.canvas.create_text(50, 210, text='0 cm'); self.canvas.create_text(600, 210, text=f"{data['length_cm']:g} cm")
            for i, side in enumerate(data['side']):
                for j in range(side['count']):
                    pos = 0.5 if side['count'] == 1 else j / (side['count'] - 1)
                    index = side['offset'] + (side['count'] - 1 - j if side['reversed'] else j)
                    r, g, b = rgb[index]; px = 50 + pos * 550; py = 80 if i == 0 else 200
                    self.canvas.create_oval(px-3, py-3, px+3, py+3, fill=f'#{r:02x}{g:02x}{b:02x}', outline='gray')
            if accepted:
                px = 50 + path / data['length_cm'] * 550
                self.canvas.create_oval(px-6, 134, px+6, 146, fill='red')
        except (ValueError, OverflowError) as e:
            self.status.set(f'Invalid input: {e}'); self.canvas.delete('all'); self.output.delete('1.0', 'end')

    def open(self):
        name = filedialog.askopenfilename(filetypes=[('JSON', '*.json'), ('All files', '*')])
        if not name: return
        try:
            with open(name, encoding='utf-8') as f: data = json.load(f)
            make_config(data, self.lib)
            self.enabled.set(data['enabled'])
            for key in (*FLOATS, *INTS): self.vars[key].set(str(data[key]))
            for i, side in enumerate(data['side']):
                for key in SIDE_FIELDS: self.vars[f'side{i}.{key}'].set(side[key])
            self.preview()
        except (OSError, ValueError, TypeError, KeyError) as e: messagebox.showerror('Cannot open configuration', str(e))

    def save(self):
        try:
            data = self.data(); make_config(data, self.lib)
            name = filedialog.asksaveasfilename(defaultextension='.json', filetypes=[('JSON', '*.json')])
            if name:
                with open(name, 'w', encoding='utf-8') as f: json.dump(data, f, indent=2); f.write('\n')
        except (OSError, ValueError, OverflowError) as e: messagebox.showerror('Cannot save configuration', str(e))

if __name__ == '__main__':
    try:
        lib = load_library(); root = tk.Tk(); App(root, lib); root.mainloop()
    except (OSError, subprocess.CalledProcessError, tk.TclError) as e:
        print(f'Could not start helper: {e}')
        raise SystemExit(1)
