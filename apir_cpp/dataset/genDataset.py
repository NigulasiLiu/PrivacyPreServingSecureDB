#!/usr/bin/python3

import os
import argparse

def rand(number_byte_len: int) -> int:
    return int.from_bytes(os.urandom(number_byte_len), 'little')

def save2txt(n: int, number_byte_len: int, filename: str):
    # 使用流式写入，避免在内存中生成巨大列表
    with open(filename, "w") as f:
        for i in range(1, n + 1):
            val = rand(number_byte_len)
            # 输出格式: "i:十六进制值" (例如 1:a2f8c)
            f.write(f"{i}:{val:x}\n")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out', type=str, help='output destination')
    parser.add_argument('--n', type=int, required=True, help='the size of dataset')
    parser.add_argument('--byte', type=int, required=True, help='the size of number in byte')
    args = parser.parse_args()
    
    n = args.n
    number_byte_len = args.byte
    filename = args.out
    
    if filename is None:
        # 后缀改为 .txt
        filename = f'./n{n}-byte{number_byte_len}.txt'
        
    save2txt(n, number_byte_len, filename)
    print(f"Dataset generated: {filename}")

if __name__ == '__main__':
    main()