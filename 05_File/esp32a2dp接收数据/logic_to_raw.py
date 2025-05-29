# -*- coding: utf-8 -*-
import struct

def hex_to_signed16(s):
    val = int(s, 16)
    if val >= 0x8000:
        val -= 0x10000
    return val

left = []
right = []

with open('0529_logic.txt', encoding='utf-8', errors='ignore') as f:
    for line in f:
        line = line.strip()
        if 'Right channel:' in line:
            hexstr = line.split(':')[-1].strip()
            right.append(hex_to_signed16(hexstr))
        elif 'Left channel:' in line:
            hexstr = line.split(':')[-1].strip()
            left.append(hex_to_signed16(hexstr))

# 确保长度相同
min_len = min(len(left), len(right))
left = left[:min_len]
right = right[:min_len]

# 交错写入 raw: L,R,L,R...
with open('output.raw', 'wb') as fout:
    for l, r in zip(left, right):
        # '<h' 表示小端16位有符号整数
        fout.write(struct.pack('<h', l))
        fout.write(struct.pack('<h', r))

print(f"YES: {min_len},OUT FILE: output.raw")