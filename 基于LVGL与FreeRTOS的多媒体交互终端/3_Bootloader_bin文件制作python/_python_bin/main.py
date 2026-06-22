import struct
import crcmod
import os
import time
import sys
import tkinter as tk
from tkinter import filedialog, messagebox

# --- 核心逻辑：CRC32 黄金法则 (严格保持不变) ---
setup_crc32 = crcmod.Crc(0x104C11DB7, initCrc=0xFFFFFFFF)

# --- 常量定义 (需与 C 语言 fw_manager.h 对应) ---
FLASH_WORD_SIZE = 32
FW_INFO_MAGIC = 0x55AA55AA

# 结构体格式定义 (小端):
# I:magic(4), 12s:ver_str(12), I:ver_num(4), I:size(4), I:crc32(4)
# B:state(1), B:cnt(1), H:rollback(2), I:ts(4), I:header_crc(4)
STRUCT_FORMAT = "<I12sIIIBBHI I"


class PackerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("多功能音乐播发器 固件适配打包工具 v1.2")
        self.root.geometry("550x450")

        # 1. 输入文件选择
        tk.Label(root, text="原始固件 (.bin):", font=('Arial', 9, 'bold')).pack(pady=5)
        self.entry_input = tk.Entry(root, width=60)
        self.entry_input.pack(padx=10)
        tk.Button(root, text="选择文件", command=self.select_file).pack(pady=5)

        # 2. 版本号输入 (字符串)
        tk.Label(root, text="固件版本字符串 (最多11字符, 如 V1.0.2):").pack()
        self.entry_ver_str = tk.Entry(root, width=20)
        self.entry_ver_str.insert(0, "V1.0.0")
        self.entry_ver_str.pack(pady=2)

        # 3. 版本号输入 (数字 - 用于比较)
        tk.Label(root, text="固件版本数字 (如 100):").pack()
        self.entry_ver_num = tk.Entry(root, width=20)
        self.entry_ver_num.insert(0, "100")
        self.entry_ver_num.pack(pady=2)

        # 4. 日志显示区域
        self.log_text = tk.Text(root, height=10, width=70, state='disabled', bg="#f4f4f4", font=('Consolas', 9))
        self.log_text.pack(pady=10)

        # 5. 开始按钮
        self.btn_pack = tk.Button(root, text="构建并保存到当前目录", bg="#4CAF50", fg="white",
                                  font=('Arial', 10, 'bold'), command=self.run_pack)
        self.btn_pack.pack(pady=10)

    def log(self, message):
        self.log_text.config(state='normal')
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.see(tk.END)
        self.log_text.config(state='disabled')

    def select_file(self):
        filename = filedialog.askopenfilename(filetypes=[("Binary files", "*.bin")])
        if filename:
            self.entry_input.delete(0, tk.END)
            self.entry_input.insert(0, filename)

    def run_pack(self):
        input_path = self.entry_input.get()
        v_str = self.entry_ver_str.get()

        # 获取当前运行环境的路径 (如果是exe则为其所在目录)
        if getattr(sys, 'frozen', False):
            current_dir = os.path.dirname(sys.executable)
        else:
            current_dir = os.getcwd()

        try:
            v_num = int(self.entry_ver_num.get())
        except ValueError:
            messagebox.showerror("错误", "版本数字必须是整数！")
            return

        if not input_path or not os.path.exists(input_path):
            messagebox.showerror("错误", "请选择有效的原始固件文件！")
            return

        # 构造输出路径：当前工作目录 + 原文件名替换
        base_name = os.path.basename(input_path).replace(".bin", "_packed.bin")
        output_path = os.path.join(current_dir, base_name)

        try:
            with open(input_path, "rb") as f:
                payload = bytearray(f.read())

            # --- 步骤 1: 32字节对齐填充 ---
            original_len = len(payload)
            padding_len = (FLASH_WORD_SIZE - (original_len % FLASH_WORD_SIZE)) % FLASH_WORD_SIZE
            if padding_len > 0:
                payload.extend(b'\xFF' * padding_len)
                self.log(f"对齐填充: {padding_len} 字节")

            # --- 步骤 2: 计算载荷 CRC32 ---
            cal_crc = setup_crc32.new(payload).crcValue
            aligned_size = len(payload)

            # --- 步骤 3: 构建 Header ---
            timestamp = int(time.time())

            # 打包前 36 字节
            header_base = struct.pack("<I12sIIIBBHI",
                                      FW_INFO_MAGIC,
                                      v_str.encode('ascii')[:11].ljust(12, b'\x00'),
                                      v_num,
                                      aligned_size,
                                      cal_crc,
                                      0, 0, 0, timestamp
                                      )

            # --- 步骤 4: 计算 Header CRC ---
            header_crc = setup_crc32.new(header_base).crcValue
            final_header = header_base + struct.pack("<I", header_crc)

            # --- 步骤 5: 写入新文件 ---
            with open(output_path, "wb") as f:
                f.write(payload)
                f.write(final_header)

            self.log(f"--- 打包成功 ---")
            self.log(f"输出路径: {output_path}")
            self.log(f"固件载荷 CRC32: {hex(cal_crc).upper()}")
            self.log(f"Header CRC: {hex(header_crc).upper()}")

            # 打包成功后尝试打开文件夹
            try:
                os.startfile(current_dir)
            except:
                pass

            messagebox.showinfo("成功", f"打包完成！\n文件已保存至：\n{output_path}")

        except Exception as e:
            messagebox.showerror("打包失败", f"发生错误: {str(e)}")


if __name__ == "__main__":
    window = tk.Tk()
    app = PackerGUI(window)
    window.mainloop()

# cd "C:\Users\16831\Desktop\项目2_Smart-H7 高性能多媒体交互终端\_python_bin2"
# & "C:\Users\16831\AppData\Local\Programs\Python\Python313\python.exe" -m PyInstaller --onefile --noconsole main.py