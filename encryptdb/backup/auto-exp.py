'''
Author: paangyubok 2837839125@qq.com
Date: 2023-05-26 17:08:37
LastEditors: paangyubok 2837839125@qq.com
LastEditTime: 2023-05-27 12:41:34
FilePath: /wlby/globcom/auto-exp.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
#!/usr/bin/python3
import subprocess
import os
import sys

def test_case(dataset_path, query_type):
    sub_ret = subprocess.check_output(["./mainExe", dataset_path, '-', query_type], cwd='./build')
    return sub_ret.decode('utf-8')

def test_cluster(n_, d_, output):
    for n in n_:
        for d in d_:
            dataset_path = os.path.abspath(f'./experiment/dataset/{n}-{d}-100000.json')
            info = test_case(dataset_path, 'r')
            output.write(f'============{dataset_path}============\n\n')
            output.write(info)
            output.write('\n--------------------------------------------------------------\n\n')
            output.flush()
            
def main():
    info = '1'
    if len(sys.argv) > 1:
        info = sys.argv[1]

    if 'N' in info:
        print("testing var n")
        n_ = [i for i in range(200, 2001, 200)]
        d_ = [3]
        with open(f'./experiment/n-var-{info}.txt', 'w') as f:
            test_cluster(n_, d_, f)
    elif 'D' in info:
        print("testing var d")
        n_ = [1000]
        d_ = [i for i in range(2, 11)]
        with open(f'./experiment/d-var-{info}.txt', 'w') as f:
            test_cluster(n_, d_, f)

if __name__ == '__main__':
    main()