#! /usr/bin/python3

import queue
import os
import random
from threading import Thread
import threading
from formatToTable import format_to_table

class Worker (Thread):
    def __init__(self, threadId, func, args = ()):
        Thread.__init__(self)
        self.threadId = threadId
        self.func = func
        self.args = args

    def run(self):
        self.func(self.threadId, *self.args)

queue_lock = threading.Lock()
work_queue = queue.Queue()
threads = []

def thread_main_func(threadId):
    while True:
        cmd = None
        queue_lock.acquire()
        if not work_queue.empty():
            cmd = work_queue.get()
        queue_lock.release()
        if cmd is None:
            break
        print(f'[{threadId}] running {cmd}')
        os.system(cmd)
    print(f'[{threadId}] thread exit')

def gen_dataset_file(n: int, num_len: int):
    # 改为 .txt 后缀
    filename = f'n{n}-byte{num_len}.txt'
    relative_path = f'../dataset/{filename}'

    # 增加大小检查，避免使用错误的 0 字节文件
    if not os.path.isfile(relative_path) or os.path.getsize(relative_path) == 0:
        os.system(f'python3 ../dataset/genDataset.py --n {n} --byte {num_len} --out {relative_path}')
    return relative_path

def generate_cmd_list(n: int, num_len: int, round: int, tag = ""):
    dataset_path = gen_dataset_file(n, num_len)
    cmd_list = []
    for i in range(1, round+1):
        output_filename = f'{n}-byte{num_len}-{i}-{tag}.txt'
        output_path = f'./output/{output_filename}'
        # PPVD 方案接收的是 dataset_path 作为参数
        cmd = f'../build/mainExe {dataset_path} > {output_path}'
        cmd_list.append(cmd)
    return cmd_list

def main():
    thread_num = 10
    round = 3

    # ====== 修改这里 ======
    # 原版：orders = [2*i for i in range(4, 11)]  # [8, 10, 12, 14, 16, 18, 20]
    # 原版：byte_sizes = [1, 2, 8]  # 数据大小（字节），对应 8, 16, 64 bit

    orders = [8, 10, 12, 14, 16, 18, 20] # 数据规模 2 的 8 到 20 次方
    byte_sizes = [1, 2]                  # 8bit 和 16bit 数据，对应 1 byte 和 2 bytes
    # =====================

    n_arr = [(2 ** i) for i in orders]
    cmd_list = []
    for num_len in byte_sizes:
        for i in range(len(n_arr)):
            cmd_list += generate_cmd_list(n_arr[i], num_len, round, f'2order{orders[i]}')

    random.shuffle(cmd_list)
    for cmd in cmd_list:
        work_queue.put(cmd)

    print (f'thread_num = {thread_num}')
    for i in range(thread_num):
        t = Worker(i, thread_main_func)
        t.start()
        threads.append(t)

    for t in threads:
        t.join()

if __name__ == '__main__':
    # 确保 output 文件夹存在
    os.makedirs("./output", exist_ok=True)
    main()
    format_to_table()