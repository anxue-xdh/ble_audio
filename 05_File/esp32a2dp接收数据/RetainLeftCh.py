# # -*- coding: utf-8 -*-

# def extract_left_channel_data(input_file, output_file):
#     left_data = []
#     with open(input_file, "r", encoding="utf-8") as fin:
#         for line in fin:
#             if "Left channel:" in line:
#                 # 提取16位音频数据
#                 parts = line.strip().split("Left channel:")
#                 if len(parts) > 1:
#                     hex_data = parts[1].strip()
#                     # 只保留合法的16进制数（通常为4或8位）
#                     if len(hex_data) == 8 or len(hex_data) == 4:
#                         left_data.append(hex_data)
#                     else:
#                         # 若格式为 '0000fe4f' 这种正常长度
#                         left_data.append(hex_data)
#     # 用空格连接并写入文件
#     with open(output_file, "w", encoding="utf-8") as fout:
#         fout.write(" ".join(left_data))

# if __name__ == "__main__":
#     extract_left_channel_data("0524_1402_i2s.txt", "left_channel_data.txt")


# -*- coding: utf-8 -*-

# -*- coding: utf-8 -*-

# def extract_both_channels_signed16(input_file, output_file):
#     left_data = []
#     right_data = []
#     with open(input_file, "r", encoding="utf-8") as fin:
#         for line in fin:
#             line = line.strip()
#             if "Left channel:" in line:
#                 hex_data = line.split("Left channel:")[1].strip()[-4:]
#                 value = int(hex_data, 16)
#                 if value >= 0x8000:
#                     value -= 0x10000
#                 left_data.append(str(value))
#             elif "Right channel:" in line:
#                 hex_data = line.split("Right channel:")[1].strip()[-4:]
#                 value = int(hex_data, 16)
#                 if value >= 0x8000:
#                     value -= 0x10000
#                 right_data.append(str(value))
#     # 交错输出，保证数据顺序（左、右长度可能不等，以最短为准）
#     min_len = min(len(left_data), len(right_data))
#     both_channels = []
#     for i in range(min_len):
#         both_channels.append(left_data[i])
#         both_channels.append(right_data[i])
#     with open(output_file, "w", encoding="utf-8") as fout:
#         fout.write(" ".join(both_channels))

# if __name__ == "__main__":
#     extract_both_channels_signed16("0524_1402_i2s.txt", "both_channel_data_signed16.txt")


# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt

def plot_both_channels(filename):
    # 读取数据
    with open(filename, "r", encoding="utf-8") as f:
        data = f.read().strip().split()
    # 转换为整数
    data = [int(x) for x in data]
    # 拆分左右声道
    left = data[::2]
    right = data[1::2]
    # 生成横坐标
    x = list(range(len(left)))
    # 绘图
    plt.figure(figsize=(12, 6))
    plt.plot(x, left, label='Left Channel')
    plt.plot(x, right, label='Right Channel')
    plt.xlabel("Sample Index")
    plt.ylabel("Amplitude")
    plt.title("Both Channels Waveform")
    plt.legend()
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_both_channels("both_channel_data_signed16.txt")