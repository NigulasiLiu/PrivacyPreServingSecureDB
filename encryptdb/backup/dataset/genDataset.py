#!/usr/bin/python3

from email.policy import default
import os
import json
import argparse

def rand()->int:
    return int.from_bytes(os.urandom(4), 'little')

def genDataset(n: int, d: int, mod: int)->list:
    return [[rand() % mod for _ in range(d)] for _ in range(n)]

def save2json(dataset: list, filename: str):
    with open(filename, "w") as f:
        json.dump(dataset, f)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out', type=str, help='output destination')
    parser.add_argument('--n', type=int, required=True, help='the size of dataset')
    parser.add_argument('--d', type=int, default=10, help='dimension')
    parser.add_argument('--T', type=int, required=True, help='domain')
    
    args = parser.parse_args()
    n = args.n
    mod = args.T
    d = args.d
    filename = args.out
    if filename is None:
        filename = f'./{n}-{d}-{mod}.json'
    save2json(genDataset(n, d, mod), filename)

main()
