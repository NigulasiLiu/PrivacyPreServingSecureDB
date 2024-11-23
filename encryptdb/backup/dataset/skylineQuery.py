#! /usr/bin/python3
import json
import argparse

def dynamic_query(filename, query):
    infile = open(filename, "r")
    dataset = json.load(infile)
    # print(dataset)
    infile.close()
    skyline = []
    skyline_idxs = []
    
    def dominate(x, y):
        ret = True
        xs = ys = 0
        for i in range(len(query)):
            if not ret: return ret
            a = abs(x[i]-query[i])
            b = abs(y[i]-query[i])
            if a < b:
                ret = False
            xs += a
            ys += b
        if xs <= ys:
            ret = False
        return ret
    
    for data in dataset:
        is_skyline = True
        remove_skyline = []
        for sky in skyline:
            if is_skyline and dominate(sky, data):
                is_skyline = False
            if dominate(data, sky):
                remove_skyline.append(sky)
        for sky in remove_skyline:
            skyline.remove(sky)
        if is_skyline:
            skyline.append(data)
    
    for sky in skyline:
        skyline_idxs.append(dataset.index(sky))
    print("dynamic skyline is ", skyline)
    print("dynamic skyline index is ", skyline_idxs)

def reverse_query(dataset_path, query):
    infile = open(dataset_path, "r")
    dataset = json.load(infile)
    infile.close()
    
    skyline = []
    skyline_idxs = []
    def dominate(x, anchor):
        ret = True
        xs = 0
        qs = 0
        for i in range(len(query)):
            if not ret: break
            tx = abs(x[i] - anchor[i])
            tq = abs(query[i] - anchor[i])
            if tx > tq:
                ret = False
            xs += tx
            qs += tq
        return ret and xs < qs
    n = len(dataset)
    for i in range(n):
        is_skyline = True
        for j in range(n):
            if i == j: continue
            if not is_skyline: break
            if dominate(dataset[j], dataset[i]):
                is_skyline = False
        if is_skyline:
            skyline.append(dataset[i])
            skyline_idxs.append(i)
    print("reverse skyline is", skyline)
    print("reverse skyline index is", skyline_idxs)            
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--f', type=str, default='./10-2-50.json', help='path to dataset')
    parser.add_argument('--q', type=list, nargs='+', required=True, help='query point')
    parser.add_argument('--t', type=str, default='r', help='type of skyline query')
    args = parser.parse_args()
    filename = args.f
    tmp_q = args.q
    query = [int("".join(i)) for i in tmp_q]
    print("query is", query)
    
    query_type = args.t
    if query_type == 'r':
        reverse_query(filename, query)
    elif query_type == 'd':
        dynamic_query(filename, query)
    else:
        print("don't have this skyline query:", query_type)