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

def gen_dataset_file(n: int):
    num_len = 4
    filename = f'n{n}-byte{num_len}.json'
    relative_path = f'../dataset/{filename}'
    if not os.path.isfile(relative_path):
        os.system(f'../dataset/genDataset.py --n {n} --byte {num_len} --out {relative_path}')
    return relative_path

def generate_cmd_list(n: int, round: int, tag = ""):
    cmd_list = []
    for i in range(1, round+1):
        output_filename = f'{n}-{i}-{tag}.txt'
        output_path = f'./output/{output_filename}'
        cmd = f'../build/mainExe {n} > {output_path}'
        cmd_list.append(cmd)
    return cmd_list

def main():
    thread_num = 10
    round = 3
    orders = [2*i for i in range(4, 11)]
    n_arr = [(2 ** i) for i in orders]
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
    