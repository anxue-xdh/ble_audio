# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt

left = []
right = []

def hex_to_signed(val):
    # """将16位无符号十六进制字符串转为有符号整数"""
    n = int(val, 16)
    if n >= 0x8000:
        n -= 0x10000
    return n

with open('0529_logic.txt', encoding='utf-8') as f:
    for line in f:
        line = line.strip()
        if 'Right channel:' in line:
            hexstr = line.split(':')[-1].strip()
            right.append(hex_to_signed(hexstr))
        elif 'Left channel:' in line:
            hexstr = line.split(':')[-1].strip()
            left.append(hex_to_signed(hexstr))

plt.figure(figsize=(18, 7))
plt.plot(right, label='Right Channel')
plt.plot(left, label='Left Channel')
plt.xlabel('Sample Index')
plt.ylabel('Amplitude (signed 16-bit)')
plt.title('I2S Logic Data (16-bit signed)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()