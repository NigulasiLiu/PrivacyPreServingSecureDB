#! /usr/bin/python3

import queue
import os
import random
from threading import Thread
import threading
import math  # 用于计算log2(n)得到x
import json  # 用于将TXT数据保存为JSON
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

def gen_dataset_file(n: int):
    # 1. 计算n对应的x（即orders中的值，如n=2^8则x=8）
    order = int(math.log2(n))  # n = 2^order，与TXT的x对应
    # 2. 定义TXT文件路径（根据实际目录结构调整相对路径）
    # 假设subsets目录与三个方案目录同级，相对路径需根据perform.py位置调整：
    # - ppvd/experiment/perform.py 相对于 subsets 的路径：../../subsets
    # - apir_cpp/experiment/perform.py 相对于 subsets 的路径：../../subsets
    # - encryptdb/experiment/perform.py 相对于 subsets 的路径：../../subsets
    txt_path = f'../../subsets/R_DBFile_{order}_128.txt'
    # 3. 定义输出JSON文件路径（保持原有dataset目录）
    json_filename = f'R_DBFile_{order}_128.json'
    json_path = f'../dataset/{json_filename}'
    
    # 4. 如果JSON不存在，则从TXT转换
    if not os.path.isfile(json_path):
        # 检查TXT文件是否存在
        if not os.path.isfile(txt_path):
            raise FileNotFoundError(f"TXT文件不存在：{txt_path}")
        # 读取TXT并转换为整数数组（与原有JSON格式一致）
        dataset = []
        with open(txt_path, 'r') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                # 解析格式："序号:十六进制字符串"，提取十六进制部分
                hex_str = line.split(':')[1]
                # 十六进制转整数（128bit数据会自动转为Python长整数）
                value = int(hex_str, 16)
                dataset.append(value)
        # 保存为JSON（与原有数据集格式兼容）
        with open(json_path, 'w') as f:
            json.dump(dataset, f)
        print(f"已将TXT转换为JSON：{json_path}")
    return json_path

def generate_cmd_list(n: int, round: int, tag = ""):
    dataset_path = gen_dataset_file(n)
    cmd_list = []
    for i in range(1, round+1):
        output_filename = f'{n}-{i}-{tag}.txt'
        output_path = f'./output/{output_filename}'
        cmd = f'../build/mainExe {dataset_path} > {output_path}'
        cmd_list.append(cmd)
    return cmd_list

def main():
    thread_num = 10
    round = 3
    orders = [2*i for i in range(4, 11)]
    n_arr = [2 ** i for i in orders]
    cmd_list = []
    for i in range(len(n_arr)):
        cmd_list += generate_cmd_list(n_arr[i], round, f'2order{orders[i]}')
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
    main()
    format_to_table()
    