#!/usr/bin/python3

import os
import json
import argparse

def rand(number_byte_len:int)->int:
    return int.from_bytes(os.urandom(number_byte_len), 'little')

def genDataset(n: int, number_byte_len: int)->list:
    return [rand(number_byte_len) for _ in range(n)]

def save2json(dataset: list, filename: str):
    with open(filename, "w") as f:
        json.dump(dataset, f)


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
        filename = f'./n{n}-byte{number_byte_len}.json'
    save2json(genDataset(n, number_byte_len), filename)

main()
