#!/usr/bin/python3
import json

def genFormatDataset(dataset:dict, n, d):
    ret = []
    count = 0
    for item in dataset:
        if count >= n: break
        record = []
        for i in range(1, d+1):
            idx = f"V{i}"
            val = str(item[idx]).replace('.', '')
            record.append(int(val))
        ret.append(record)
        count+=1
    return ret

def genDatasetCluster(dataset, n_:list, d_:list):
    for i in n_:
        for j in d_:
            with open(f'./{i}-{j}-100000.json', 'w') as f:
                json.dump(genFormatDataset(dataset, i, j), f)
    

def main():
    dataset_path = './eeg-eye-state.json'
    djson = open(dataset_path, 'r')
    dataset = json.load(djson)
    djson.close()
    
    n_ = [1000]
    d_ = [i for i in range(11, 15)]
    
    # n_ = [i for i in range(200, 2001, 200)]
    # d_ = [3]
    # genDatasetCluster(dataset, n_, d_)
    
    # n_ = [1000]
    # d_ = [i for i in range(2, 11)]
    genDatasetCluster(dataset, n_, d_)
    print('done')
    
if __name__ == '__main__':
    main()